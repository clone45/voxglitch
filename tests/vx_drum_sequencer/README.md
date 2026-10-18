# tests/vx_drum_sequencer

Out-of-Rack tests for the VX Drum Sequencer engine (`VXDrumSequencerEngine.hpp`,
which is Rack-free; see `docs/modules/vx-drum-sequencer/chain-foundations.md`).
The plugin itself is never built in this environment; these compile the
engine directly. The engine pulls in the shared reset helper
`src/vgLib-2.0/sequencer/ResetClockWindow.hpp` by a relative include, so the
one `-I` below is still enough.

    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o types_test  types_test.cpp  && ./types_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o engine_test engine_test.cpp && ./engine_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o shaper_test shaper_test.cpp && ./shaper_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o chain_test  chain_test.cpp  && ./chain_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o tweak_test  tweak_test.cpp  && ./tweak_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o seq3_parity_test seq3_parity_test.cpp && ./seq3_parity_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o tweak_ladder tweak_ladder.cpp && ./tweak_ladder   # not a test: prints the ladders

- **types_test.cpp** — the per-pad model (`Step`, `Memory`, `Bank`): the
  defaults a fresh or cleared memory carries (off, single, chance 100),
  elementwise equality down to the last pad, and SEED.
- **engine_test.cpp** — `Playhead` (step 1 current from the start, advance
  and wrap, a shrunken length, `reset()` to 0, `rewind()` to -1 for the
  hand-off), `ClockFollower` (anchor before measure, the partial interval
  after a rearm, the 1-sample / 4 s window, rescale), `resolveStep` (mutes,
  the accent lane, out-of-range positions), chance (100 always, 0 never, 50
  and 10 within tolerance over 4000 rolls, a failed roll arms no ratchets and
  flashes nothing, a seeded `Rng` is deterministic), `Ratchets` (no period no
  re-strikes, x2 / x4 spacing, the ACC bit, the 2-sample floor, cancel),
  `Sequencer::process` end to end for a lone module (load, a reset firing
  step 1 with its pulse, the clocks after it as steps 2 3 4 and the wrap,
  ratchets across a step), and **the reset rule**, the VCV standard, one case
  per row of `rack-port-design.md` §4.3: load is an implicit reset (step 1
  current, nothing fires, window armed), a reset fires step 1 on that
  sample, a clock inside the window is dropped, the exact integer window
  (at 48 kHz a clock 47 samples after a reset is swallowed and 48 samples
  after plays step 2; at 44.1 kHz the window is 45 samples), same-sample
  reset + clock fires once, reset-then-clock a sample later the same,
  clock-then-reset a sample later fires N+1 then step 1, a reset in the
  clock's high half, a mid-period reset with ratchets cancelled and re-armed
  from the surviving period, a re-armed window, clocks faster than the
  window, a reset with no memory, and the hand-off arming no window.
- **shaper_test.cpp** — `TriggerShaper` as voltage tables: pulse length,
  the gap rule (a re-strike on a high lane, the exact-boundary case, the
  gap applied to the whole strike set, no gap for an idle lane), and the
  ACC cut (on the rise, after a gap, an accented set keeping its own gate).
  Each case is one paragraph of the comment above `TriggerShaper`.
- **chain_test.cpp** — three banks, ONE `Sequencer`, and a `Head` that
  mirrors the module's chain logic (the load's implicit reset, RST before
  CLK, a pending hand-off taken on the next accepted clock edge before the
  advance, never on a reset sample and never on an edge the window
  swallows, the reset played through `seq.reset()`, the hand-off decided
  whenever the active member is on its last step, put there by a fire or by
  an implicit reset).
  Load as an implicit reset, step order across members and the wrap to the
  head, a lone member looping, a reset returning to the head and playing its
  step 1 at once, a length change under the playhead, ratchets finishing
  across a hand-off, a member removed from under the playhead, a one-step
  head after an implicit reset (load, and a member removed: the first
  accepted clock plays member 1's step 1, as a flat sequencer of the same
  total length would play its step 2, not the head's step 1 again; a chain
  of one is untouched), and a clock
  one sample after a reset on a one-step head (swallowed, so it takes no
  hand-off: the head stays on step 1 and the next accepted clock plays
  member 1's step 1). Scenarios
  that need member 0's step 1 to have played send the master's reset first
  (`run(h, n, true)`), so the expected fire lists read as they always did.
  **`Head` is a copy of `VXDrumSequencer::process()` steps 3-7 — if those
  change, re-sync it.**
- **seq3_parity_test.cpp** — the reset rule against Fundamental SEQ3. A
  model of SEQ3's reset/clock/index logic with Rack's `SchmittTrigger` and
  `PulseGenerator` copied verbatim (float arithmetic included) and this
  engine behind the module's adapter are driven by the same deterministic
  xorshift32 edge streams (four seeds x 250 000 samples: same-sample resets,
  a sample before, a sample after, resets in the clock's high half, re-armed
  windows, clocks faster than the window; reset-to-clock gaps of 45..51
  samples are avoided because that is SEQ3's own float boundary), and the
  playhead must equal SEQ3's index after every sample. Then the
  Impromptu-Clocked-at-defaults scenarios the sync report was about: stop /
  reset / run (no edge at run start, beat 2 is step 2), a reset in the
  clock's high half while running, and a same-sample reset + clock.
- **tweak_test.cpp** — `tweakMemory` invariants (level 0 is the base, one
  pad per level per lane, lanes outside the mask and steps past the length
  untouched, the kick on the 1 survives, soft adds carry the soft chance) and
  `tweakLevelFromVolts` (range, big jumps, the hysteresis at a boundary).
- **tweak_ladder.cpp** — not a test: prints levels 0-8 of every lane's TWEAK
  ladder for a few base patterns as text grids, with the reason for each
  level. The tool for tuning the vocabulary in `VXDrumSequencerTweak.hpp`.
