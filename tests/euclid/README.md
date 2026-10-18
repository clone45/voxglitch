# tests/euclid

Out-of-Rack tests for the Euclid engine (`src/modules/Euclid/EuclidEngine.hpp`,
which is Rack-free; see `docs/implementation_plans/euclid-migration.md` §5.1
and §9). The plugin itself is never built in this environment; these compile
the engine directly. `-I ../../src` is what resolves the engine's include of
`vgLib-2.0/sequencer/ResetClockWindow.hpp`, the shared reset rule.

    g++ -std=c++11 -O2 -I ../../src -o pattern_test     pattern_test.cpp     && ./pattern_test
    g++ -std=c++11 -O2 -I ../../src -o engine_test      engine_test.cpp      && ./engine_test
    g++ -std=c++11 -O2 -I ../../src -o seq3_parity_test seq3_parity_test.cpp && ./seq3_parity_test

- **pattern_test.cpp** — the Bresenham pattern and the CV law. The golden
  table from the plan's §2, including E(5,8) = `x.x.xx.x`, the row that
  tells Bresenham from Bjorklund (`x.xx.xx.`); rotation as the cyclic shift
  `base[(i + rot) % s]` for every (h, s, r), and its wrap (rot 31 on 8 steps
  is rot 7, a negative rot comes into range); the clamps (hits > steps is all
  hits, hits 0 all rests, steps 1, steps 0 / 40 clamped, `hit[]` false beyond
  steps); every (h, s) in 0..32 x 1..32 has exactly min(h, s) hits; the
  dirty cache (`compute` returns true only when it recomputed, and caches the
  clamped triple); and `effectiveValue`'s truncation toward zero (1 V is +3,
  -0.5 V is -1, atten 0 silences the jack, atten -1 inverts, the clamps).
- **engine_test.cpp** — `Sequencer::process` under the VCV reset standard,
  one case per row of the plan's §4.1 table at 48 kHz (a 48-sample window
  passed as a literal): load leaves step 1 current with GATE high and no
  strike, and swallows a clock inside its window; a reset strikes step 1 on
  that sample and arms the window (never counts as a clock, never blocked by
  the window, re-arms to full); clocks at +1 and +47 are dropped and +48
  advances to step 2; a same-sample reset and clock strikes once; a clock
  followed by a reset on the next sample strikes N+1 then step 1 (the
  standard's documented limitation); a reset while GATE is high reports the
  one-sample `gate_gap`; wrap at the effective length; STEPS shrinking below
  the playhead reads GATE low then wraps on the next clock; ROT changed
  between clocks is read on the next edge (and on the current step at once);
  GATE holds across two consecutive hit steps with `struck` only on the edge
  samples; `struck` is false on every non-edge, non-reset sample; and the
  window length is whatever the caller passed (45, 1, 96).
- **seq3_parity_test.cpp** — a float model of Fundamental SEQ3's reset/clock
  logic (`dsp::SchmittTrigger`, `dsp::PulseGenerator`, `clock && !resetGate`)
  and `euclid::Sequencer` behind the same Schmitts, driven by the same
  xorshift32-generated reset/clock voltages, with the contract's 48-sample
  window (three seeds x 250 000 samples:
  same-sample reset+clock, clock one sample after reset, reset one sample
  after clock, clocks inside and just outside the window, resets inside a
  window, bursts, long idles) with `position == index` checked after every
  sample. Reset-to-clock gaps avoid the band around SEQ3's 48th sample, its
  float boundary (measured, SEQ3 closes on our sample there, but the margin is
  too thin to promise for every compiler). Then the Impromptu Clocked defaults
  (a stopped reset with the clock line held high, no edge at run start) and a
  running reset inside the clock's high half, against the expected step
  schedule and each other. Last, the exact boundary: `samplesFor` at 48 kHz,
  44.1 kHz and 96 kHz; a clock 47 / 48 samples after a reset (44.1 kHz:
  44 / 45) swallowed / accepted by us and, at those two rates, by SEQ3's float
  model on the same samples; and 96 kHz, where SEQ3's float pulse runs 97
  samples against our 96, so a clock exactly 96 samples after a reset is the
  one sample the two disagree on (95 and 97 agree). Prints the measured SEQ3
  spans at 44.1 / 48 / 96 / 192 / 384 kHz next to `samplesFor`, and the
  number of samples compared.

The compiled binaries are built on demand and are not committed (`.gitignore` here
lists them).
