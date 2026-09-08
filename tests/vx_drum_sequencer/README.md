# tests/vx_drum_sequencer

Out-of-Rack tests for the VX Drum Sequencer engine (`VXDrumSequencerEngine.hpp`,
which is Rack-free; see `docs/modules/vx-drum-sequencer/chain-foundations.md`).
The plugin itself is never built in this environment; these compile the
engine directly.

    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o types_test  types_test.cpp  && ./types_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o engine_test engine_test.cpp && ./engine_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o shaper_test shaper_test.cpp && ./shaper_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o chain_test  chain_test.cpp  && ./chain_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o tweak_test  tweak_test.cpp  && ./tweak_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o tweak_ladder tweak_ladder.cpp && ./tweak_ladder   # not a test: prints the ladders

- **types_test.cpp** — the per-pad model (`Step`, `Memory`, `Bank`): the
  defaults a fresh or cleared memory carries (off, single, chance 100),
  elementwise equality down to the last pad, and SEED.
- **engine_test.cpp** — `Playhead` (advance-then-play from -1, wrap, a
  shrunken length), `ClockFollower` (anchor before measure, the partial
  interval after a rearm, the 1-sample / 4 s window, rescale),
  `resolveStep` (mutes, the accent lane, out-of-range positions), chance
  (100 always, 0 never, 50 and 10 within tolerance over 4000 rolls, a failed
  roll arms no ratchets and flashes nothing, a seeded `Rng` is deterministic),
  `Ratchets` (no period no re-strikes, x2 / x4 spacing, the ACC bit, the
  2-sample floor, cancel), and `Sequencer::process` end to end for a lone
  module (first edge fires step 1 on that sample, pulse length, wrap, a
  reset followed by a same-sample clock, ratchets across a step).
- **shaper_test.cpp** — `TriggerShaper` as voltage tables: pulse length,
  the gap rule (a re-strike on a high lane, the exact-boundary case, the
  gap applied to the whole strike set, no gap for an idle lane), and the
  ACC cut (on the rise, after a gap, an accented set keeping its own gate).
  Each case is one paragraph of the comment above `TriggerShaper`.
- **chain_test.cpp** — three banks, ONE `Sequencer`, and a `Head` that
  mirrors the module's chain logic (a pending hand-off taken on the next
  clock edge, before the advance). Step order across members and the wrap
  to the head, a lone member looping, reset returning to the head, a
  length change under the playhead, ratchets finishing across a hand-off,
  a member removed from under the playhead. **`Head` is a copy of
  `VXDrumSequencer::process()` steps 4-7 — if those change, re-sync it.**
- **tweak_test.cpp** — `tweakMemory` invariants (level 0 is the base, one
  pad per level per lane, lanes outside the mask and steps past the length
  untouched, the kick on the 1 survives, soft adds carry the soft chance) and
  `tweakLevelFromVolts` (range, big jumps, the hysteresis at a boundary).
- **tweak_ladder.cpp** — not a test: prints levels 0-8 of every lane's TWEAK
  ladder for a few base patterns as text grids, with the reason for each
  level. The tool for tuning the vocabulary in `VXDrumSequencerTweak.hpp`.
