# Euclid — VCV Rack Port Design

Where the Rack port of the vxsynth **Euclidean Sequencer** (`Euclid Seq`) diverges
from its source, and why. Read alongside `user_manual.md` in this directory. The
port ships as one standalone module, `Euclid`, whose behaviour contract is the
vxsynth WASM module `euclid.c`, whose display is the vxsynth Ring widget, and whose
Rack idioms (thresholds, pulse lengths, panels, persistence, tests) follow the
conventions set by the VX Drum Sequencer port, except reset, which follows the VCV
standard (§4.1).

Source citations are to `~/vxsynth` as of 2026-09-16, the day the port was
specified. `§n` references without a document name are to this record.

**Status:** design bound, 2026-09-16. Every behaviour below is either a decision the
owner took on that date (§1) or copied from `euclid.c` unchanged (§3). Nothing here is
provisional. The reset rule in §4.1 is the VCV standard, not the rule the VX Drum
Sequencer port shipped with; that module is being moved to the same standard in the
same change (§4.6), through the shared helper both engines now call.

**2026-09-18: GATE dropped, 9 HP, one jack row.** The owner removed the GATE output
(TRIG is the module's only jack; the engine still computes the gate level and its
tests still pin it, nothing reaches a jack), widened the panel to 9 HP, moved the
EUCLID frame to the right of the wordmark, put CLK / RST / TRIG on one bottom row and
gave the freed height to the ring, now 117 px square. §1 row 2, §3.4, §5.2 and §7 are
revised; §4.1's GATE sentences describe the engine's `gate` / `gate_gap` fields, which
survive unchanged.

---

## 1. Decisions locked

| # | Decision | Value |
|---|---|---|
| 1 | Name and slug | Module name **Euclid**, slug `euclid`, `panelName` on the frame **EUCLID** (the vxsynth label). The slug is Rack's identity for the module and never changes once a version ships |
| 2 | Width | **9 HP** (135 px) since 2026-09-18 (8 HP at first). Three columns of knobs, trimpots and CV jacks at a 45 px pitch, one bottom row of three jacks (CLK, RST, and TRIG on the outputs plate), a 117 px ring. The EUCLID frame sits beside the wordmark, not under it |
| 3 | Attenuverters | **Kept** (the one thing vxsynth added to GAEuclid). Three bipolar trimpots, one per CV jack |
| 4 | Attenuverter default | **0**: a patched jack is silent until its trimpot is turned up, for parity with vxsynth patches and the vxsynth manual. Flagged plainly in the user manual, since a patched-but-silent jack is a classic support question |
| 5 | The ring | **A control**, as in vxsynth: rim drag rotates, double-click zeroes. One param written through its `ParamQuantity` with a history entry per drag |
| 6 | What the ring draws | **The effective pattern** (knobs plus CV), as GAEuclid did. In Rack there is nothing else that shows what the CV is doing. The knobs show knob positions |
| 7 | Toussaint polygon | **No.** The vxsynth look is the target. Trivial to add later |
| 8 | Reset convention | **The VCV standard** (§4.1): a reset makes step 1 current and plays it; clock edges are ignored for 1 ms after a reset; the next clock plays step 2. Not the VX Drum Sequencer port's `position = -1` rule, which is the "offset" family the sync complaint is about |
| 9 | Trigger length | **1 ms** default, context-menu list `{1, 2, 5, 10, 20, 50, 100, 200} ms` as ArpSeq and VX Drum Sequencer, persisted as an index |
| 10 | Engine | **Rack-free engine with out-of-Rack tests**, as VX Drum Sequencer. The tests pin Bresenham and the reset cases |
| 11 | VX Drum Sequencer | **Moved to the same standard** (§4.6). The reset-and-window logic is one shared header, `src/vgLib-2.0/sequencer/ResetClockWindow.hpp`, that both engines call, so the standard is written once |

---

## 2. Provenance

The vxsynth module is itself a port. Its DSP header says so: *"Ported faithfully from
Groovebox Advanced GAEuclid (GAEuclidDSP)"*. There are three copies of the same
algorithm, and the oldest one is already Rack C++:

| Copy | Where | Notes |
|---|---|---|
| **GAEuclid** (origin) | `/mnt/c/Code/voxglitch_devices/src/GrooveboxAdvanced/modules/GAEuclid.hpp` | An internal module of Groovebox Advanced in the *Voxglitch Devices* plugin. Rack C++, NanoVG ring overlay with a Toussaint polygon. No attenuverters. Not a standalone module |
| **euclid.c** | `~/vxsynth/app/studio/lib/audio/wasm/modules/euclid.c` | The WASM port. Byte-for-byte the same Bresenham pattern and step logic; **adds three bipolar attenuverters** on the CV jacks |
| **euclid.js / panel-ring.js** | `~/vxsynth/app/studio/lib/modules/definitions/euclid.js`, `.../canvas/floating-panel/panel-ring.js` | The panel definition and the Ring widget (rim-drag rotation, double-click reset, live playhead) |
| manual | `~/vxsynth/public/studio/docs/euclid.md` | The seed for `user_manual.md` |

So the port is not "translate C to C++". It is a new standalone Voxglitch module whose
**behaviour contract is euclid.c** (the vxsynth feature set, attenuverters included) and
whose **display is panel-ring.js**. GAEuclid.hpp is a NanoVG reference for the ring,
nothing more.

**Bresenham, not Bjorklund.** The source computes the pattern with a Bresenham line
walk, not Bjorklund's algorithm. They produce the same rhythms up to rotation, but not
the same rotation: E(5,8) is `x.x.xx.x` here, where Bjorklund gives `x.xx.xx.`. The
Bresenham output is the contract (vxsynth patches and the GA module depend on it) and
the tests in §8 pin it. Anyone tempted to "correct" the algorithm should read this
paragraph twice.

---

## 3. The contract

Everything in this section is `euclid.c` as is.

### 3.1 Parameters

| Param | Range | Default | Notes |
|---|---|---|---|
| HITS | 0..32, integer | 4 | clamped to STEPS at use |
| STEPS | 1..32, integer | 16 | |
| ROT | 0..31, integer | 0 | clamped to `0..steps-1` at use (§3.2) |
| HITS CV atten | -1..1 | 0 | bipolar; 0 = jack ignored |
| STEPS CV atten | -1..1 | 0 | |
| ROT CV atten | -1..1 | 0 | |

**Jacks**: CLK, RST, HITS CV, STEPS CV, ROT CV in; TRIG out (GATE was dropped on 2026-09-18). Mono only.

### 3.2 CV law

`effective = clamp(knob + (int)(cv * 3.2 * atten), lo, hi)`. The `(int)` truncates
toward zero, so ±5 V is ±16 steps and 1 V is 3 steps, not 4. Hits are then clamped to
the effective step count; rotation to `0..steps-1`.

### 3.3 Pattern

`euclid_recompute_pattern`, dirty-cached on the three effective ints:

```
base[0] = 1
base[i] = floor(i*h/s) > floor((i-1)*h/s)      for i in 1..s-1   (integer division)
pattern[i] = base[(i + rot) % s]
h >= s  -> all hits;   h == 0 -> all rests
```

Golden values for the tests (§8), computed from that rule:

| (h, s) | pattern |
|---|---|
| (3, 8) | `x..x..x.` |
| (5, 8) | `x.x.xx.x` |
| (4, 16) | `x...x...x...x...` |
| (5, 16) | `x...x..x..x..x..` |
| (3, 8) rot 1 | `..x..x.x` (increasing ROT pulls hits to *earlier* indices) |

### 3.4 Outputs

TRIG is a 10 V pulse raised when the playhead lands on a hit, 0 V on rests. The
source's GATE (10 V for the whole of any hit step, consecutive hits one continuous gate)
is computed by the engine as `Out::gate` but has had no jack since 2026-09-18.

### 3.5 Playhead

Advances one step per clock rising edge, modulo the effective step count, so a STEPS
change that shrinks below the playhead wraps on the next clock. Step 1 (index 0) is the
current step after load and after reset, and GATE reflects it, so GATE is high at rest
whenever step 1 is a hit (with ROT 0 and HITS > 0, always). That is the source's
behaviour and, as §4.1 shows, the standard's.

---

## 4. Divergences from euclid.c

### 4.1 Reset and the first clock: the VCV standard

**What the standard says.** The VCV Rack manual's *Voltage Standards* page
(https://vcvrack.com/manual/VoltageStandards) is explicit:

> Each cable in Rack induces a 1-sample delay of its carried signal from the output
> port to the input port. [...] a pulse sent through a utility module and then to a
> sequencer's CLOCK input will arrive one sample later than the same pulse sent
> directly to the sequencer's RESET input. This will cause the sequencer to reset to
> step 1, and one sample later, advance to step 2, which is undesired behavior.
> Therefore, modules with a CLOCK and RESET input, or similar variants, should ignore
> CLOCK triggers up to 1 ms after receiving a RESET trigger.

Note the model that sentence assumes: **a reset puts the sequencer ON step 1**, and the
clock that accompanies it must be swallowed, because otherwise it would advance to
step 2. The next real clock is step 2's.

**What the reference implementations do.**

- *Fundamental SEQ3* (VCV's own, `src/SEQ3.cpp`): on a reset edge, `index = 0`,
  `phase = 0`, `resetPulse.trigger(1e-3f)`; the clock branch is
  `if (clockTriggered && !resetGate) clock = true;`, so any clock edge inside the 1 ms
  reset pulse is dropped; reset is processed **before** the clock in `process()`; and
  because `clockPulse.trigger(1e-3f)` fires whenever `index != oldIndex`, **a reset
  that moves the index fires step 1's trigger at the reset instant.** The next clock
  advances to step 2.
- *Impromptu Modular* (Clocked, PhraseSeq, GateSeq64, Foundry; README and the
  published `README_SeqCode` reference): "all incoming clock edges are ignored during
  1ms after the reset was received. The clock is also ignored during 1ms when the
  module is initialized or powered up (which is treated as an implicit reset)."
  Additionally "all gate outputs are forced low during 1ms after the reset edge, to
  ensure any trigger-sensitive targets are effectively triggered."
- *Clocked's defaults* assume that model: "the clock outputs are held high when a
  stopped clock is reset [...] such that when starting again, it will not miss the
  first step in sequential switches or sequencers that don't have run inputs." That
  is: the master expects the reset itself to have put every sequencer on step 1, and
  deliberately produces **no clock edge** at run start. A reset while running behaves
  the same way whenever it lands in the clock's high half; Count Modula, in the thread
  below: "If you reset whilst the output is high, it simply restarts the cycle and
  leaves the output high so there is no positive edge presented to the sequencers."
- *The community thread "Reset Sequencers are one step off"*
  (https://community.vcvrack.com/t/reset-sequencers-are-one-step-off/12898) sorted
  sequencers into two families. **In sync:** VCV SEQ-3, AS SEQ-16, "reset to step 1,
  play step 2 on next clock". **Offset:** JW Trigs, LedSeq, Basic Sequencer, "reset to
  no active step, play step 1 on next clock". The offset family is one step behind
  after any reset that is not accompanied by a clock edge. Voxglitch's Digital
  Sequencer is named in that thread as needing its "Reset Mode" menu switched (to
  "Instant", the standard behaviour) before it would line up.

**The rule for this module**, which is SEQ3's, and which is written once in
`src/vgLib-2.0/sequencer/ResetClockWindow.hpp` (`vgLib_v2::ResetClockWindow`; its
header comment restates the standard and the per-sample order below):

1. **RST rising edge** (Schmitt, `constants::gate_low_trigger` / `gate_high_trigger`,
   0.1 V / 2.0 V): `position = 0`. Step 1 is current: GATE follows `pattern[0]`, and
   if step 1 is a hit **TRIG fires now**, whether or not the playhead moved: unlike
   SEQ3, which fires only when `index != oldIndex`, a reset that lands while Euclid is
   already on step 1 still fires TRIG, as Impromptu's sequencers do (the drum
   sequencer's record §4.3 makes the same call), and the parity test compares
   positions, not triggers. If GATE was already high, it is forced to 0 V
   for one sample so a downstream envelope sees a fresh edge (Impromptu's gate
   retrigger, at the drum sequencer's one-sample width instead of 1 ms). A **1 ms
   clock-ignore window** is armed: `window.arm(ResetClockWindow::samplesFor(sampleRate))`,
   where `samplesFor` is `sampleRate / 1000` rounded up (48 at 48 kHz, 45 at 44.1 kHz;
   integer arithmetic, so no float overshoot) and never fewer than one sample,
   computed when armed, never cached. A reset inside an open window re-arms it to the
   full length; it never shortens.
2. **CLK rising edge** while `window.open()`: ignored. Otherwise: advance-then-play,
   `position = (position + 1) % steps`, TRIG if a hit, GATE from the pattern.
3. **Order inside one sample:** RST, then CLK, then `window.tick()`, then outputs. The
   arm on the reset sample and the same-sample clock check both see the window open,
   so a same-sample reset and clock plays step 1 once, at the reset. The window covers
   the reset sample plus `ceil(1 ms) - 1` more: 48 samples at 48 kHz, 45 at 44.1 kHz.
   That is the span of SEQ3's float `resetPulse.trigger(1e-3f)` (read through
   `PulseGenerator::process`) at 44.1 and 48 kHz and at every other Rack rate but
   three: at 96, 192 and 384 kHz SEQ3's float runs one sample longer (97 / 193 / 385),
   so the boundary sample there is the one place the two can differ (measured at all
   fifteen Rack rates; `tests/euclid/seq3_parity_test.cpp` pins the 96 kHz case).
4. **Patch load and Rack's Initialize** are implicit resets (Impromptu): `position = 0`,
   GATE from the pattern, window armed, **no TRIG** (`Euclid.hpp` `onReset()` calls the
   same `seq.load()` as the first `process()`).
5. The Schmitt triggers are never `reset()`: a line that is high is high, not an edge.

**Where the source stands.** `euclid.c` (and GAEuclid before it) is a hybrid of the two
families: reset makes step 1 current (GATE goes high at the reset), but TRIG waits for
the next clock, which is then *not* an advance (`firstStep`), and on top of that a
**10 ms** clock-ignore window is armed. With a master that sends reset and clock
together, the window swallows that clock and `firstStep` swallows the next: step 1's
trigger lands a whole beat late, and with a master that sends reset alone, the source
is in the offset family. Both mechanisms go; the 10 ms window is ten times the
standard and would swallow real clocks above 100 Hz (1/64 notes at 94 BPM, or any
multiplied clock).

**What happens, case by case:**

| Situation | Result |
|---|---|
| Reset and clock edge on the same sample (Clocked restarting its cycle) | Step 1 plays once, at the reset. The clock is swallowed. Next edge: step 2 |
| Reset direct, clock one sample later (clock routed through a utility) | Same: step 1 at the reset, the late clock is inside the window |
| Clock direct, reset one sample later (reset routed through a utility) | Step N+1 fires, then step 1 one sample later: two hits. The standard's known limitation; SEQ3 does the same. The manual's advice is to delay reset and clock equally |
| Reset while the clock output is high (no edge from the master) | Step 1 plays at the reset; the next rising edge is step 2. In sync with SEQ3, AS and Impromptu |
| Reset while stopped, Clocked default ("outputs high on reset when not running") | Step 1 plays at the reset; run start produces no edge; beat 2 is step 2. In sync |
| Mid-period reset, free-running clock | Step 1 plays at the reset and gets the remainder of the period; next edge is step 2 (Impromptu's rule) |
| Reset while GATE is high | GATE drops for one sample, then follows step 1 |
| Clock faster than 1 kHz | Edges within 1 ms of a reset are lost. Acceptable; that is the standard's window |
| Reset while already on step 1 | Step 1 plays again: TRIG fires (SEQ3 would be silent here; Impromptu fires) |
| Patch load, Rack's Initialize | Step 1 current, GATE shows it, no TRIG, window armed |
| STEPS shrinks below the playhead | GATE reads a cleared slot (0 V) until the next clock wraps to step 1 |

### 4.2 Trigger pulse

The source fires a fixed **48 samples** (1 ms only at 48 kHz). The port computes the
pulse from `args.sampleRate` and the menu length (decision 9). `dsp::PulseGenerator`
only extends a pulse that is already high, so a clock faster than the pulse length
would merge consecutive hits; the port uses the VX Drum Sequencer rule instead (a
sample countdown, with one forced 0 V sample when a new hit lands while TRIG is still
high). Single lane, so it is a dozen lines, not the whole `TriggerShaper`. The same
one-sample gap serves the GATE retrigger in §4.1.

### 4.3 Sample-rate independence

The rate-dependent quantities are the trigger pulse (§4.2) and the 1 ms reset window
(§4.1); both are computed from `args.sampleRate` at the moment they are armed, so
`onSampleRateChange` has no work to do. There is no measured clock period here (no
ratchets, no swing).

### 4.4 Parameter reading

Knobs, attenuverters and CV are read every sample, as in the source; the pattern is
recomputed only when one of the three effective integers changes (the source's dirty
cache), so the per-sample cost is three multiplies and a compare. No `ClockDivider`
needed. Knobs are configured with `snapEnabled = true` and integer display; the
attenuverters get `%` display like the other Voxglitch trimpots.

### 4.5 What is dropped

- The source's **10 ms window** and its **`firstStep` clock absorb** (§4.1), replaced
  by the standard's 1 ms window with step 1 current at the reset.
- The source's `> 1.0 V` edge detection, replaced by the house Schmitt thresholds.
- The `report` channel and `syncFromDSP` machinery: in Rack the widget reads the
  module directly (§5.3).
- The vxsynth panel's "Attenuverters" group frame is a panel-art matter, not code.
- GAEuclid's Toussaint polygon (decision 7).

### 4.6 VX Drum Sequencer moves to the same standard

The research in §4.1 explained a live complaint against a module already shipped, and
the owner's decision 11 is to fix that module first so that Euclid inherits a tested
reset idiom rather than introducing a second one.

- **VX Drum Sequencer** shipped in the *offset* family by design, as its
  `rack-port-design.md` bound it on 2026-09-01: `position = -1` after a reset, nothing
  fires at the reset instant, no clock-ignore window, "the next clock plays step 1".
  (That record's §1 "Reset" and §4.2 have since been revised to the standard; the old
  rule and its replacement are set out in its 2026-09-16 changelog paragraph, above
  its §1, and in its §4.1 row 1.)
  That is exactly the family the community thread found one step behind. With Clocked
  at its defaults it is one step behind after every stop-reset-run (no edge at run
  start, so its step 1 waits for beat 2) and after every running reset that lands
  while the clock output is high. That is the reported "does not sync" behaviour. The
  port's design record reasoned from the source's 1 ms window swallowing a coincident
  clock and concluded "no window, ever"; the standard's answer to the same problem is
  the opposite one: keep the window, and make the reset itself play step 1.
- **The change**, in its engine: `Sequencer::reset()` (an RST edge or the RWD button)
  rewinds to step 1, arms the shared window and fires step 1 through `fire()`
  (ratchets arm if a period is known); `Sequencer::rewind()` is the same without the
  fire, used for every implicit reset (load, becoming the head, a member leaving from
  under the playhead); `process()` drops clock edges while the window is open.
  The chain **hand-off** keeps its "-1 then advance" shape: the next member's step 1
  is played by the next clock edge, not by the hand-off. Its `engine_test.cpp` and
  `chain_test.cpp` reset cases are rewritten to the standard, and the case table in
  its `rack-port-design.md` §4.3 is replaced by the one in §4.1 above. That work is
  recorded in the drum sequencer's own design record; this one only notes that the two
  modules now agree.
- **The shared helper** is `vgLib_v2::ResetClockWindow`
  (`src/vgLib-2.0/sequencer/ResetClockWindow.hpp`): `samplesFor(sampleRate)`,
  `arm(samples)`, `open()`, `tick()`, `clear()`. It is Rack-free, so both engines'
  out-of-Rack tests compile it directly.

For the record, the other sequencers in the plugin as they stand today, none of which
this port changes: **ArpSeq** is advance-then-play from -1 with the window removed in
#278 (commit 30dcc68), the offset family; its `legacy_reset_mode` is the nearer thing to
the standard but does not play step 1 at the reset. **Digital Sequencer / XP** offer
"Reset Mode: Next clock input" (default, offset family) and "Instant" (standard), and
arm a **10 ms** window in both, ten times the standard; GlitchSequencer and
AutobreakStudio carry the same window. Aligning them is a ticket of its own, with a
legacy option wherever a shipped patch could depend on the old timing.

---

## 5. Architecture

Mirrors VX Drum Sequencer: a Rack-free engine, a thin Rack adapter, a panel widget, and
one custom display widget.

```
src/modules/Euclid.cpp                      the TU: includes, using namespace, createModel
src/modules/Euclid/EuclidEngine.hpp         Rack-free: effectiveValue, Pattern, Sequencer::process
src/modules/Euclid/Euclid.hpp               Module: params/inputs/outputs, Schmitts, pulse, JSON
src/modules/Euclid/EuclidRingWidget.hpp     the ring display + rotation control
src/modules/Euclid/EuclidWidget.hpp         ModuleWidget: PanelHelper anchors, menu
src/vgLib-2.0/sequencer/ResetClockWindow.hpp  the shared reset/clock window (§4.1, §4.6)
src/plugin.hpp                              extern Model* modelEuclid;
src/plugin.cpp                              p->addModel(modelEuclid);
plugin.json                                 slug, name, description, tags ["Sequencer"], manualUrl
res/modules/euclid/euclid_panel.svg         generated (§7)
res/modules/euclid/euclid_panel-dark.svg    generated
scripts/generate_euclid_panel.py            the generator: the Euclid geometry table
scripts/panel_gen/                          the shared panel package it imports (§7)
docs/modules/euclid/user_manual.md          seeded from vxsynth's euclid.md
docs/modules/euclid/rack-port-design.md     this document
tests/euclid/{README.md, .gitignore, check.hpp, pattern_test.cpp, engine_test.cpp, seq3_parity_test.cpp}
README.md                                   module list entry
```

`Makefile` needs nothing: `src/modules/*.cpp` is globbed.

### 5.1 The engine (`namespace euclid`)

The engine is pure integers and bools; it compiles standalone for the tests. The
header is the authority on names; this is its shape:

```cpp
static const int MAX_STEPS = 32;

int effectiveValue(int knob, float cv, float atten, int lo, int hi);   // §3.2 CV law

struct Pattern {
    bool hit[MAX_STEPS];             // false beyond `steps`
    int hits, steps, rot;            // the clamped triple the table was computed from
    bool compute(int h, int s, int r);   // §3.3 Bresenham; dirty-cached, true only when it recomputed
    int  count() const;              // hits in the table: min(hits, steps)
};

struct Sequencer {
    Pattern pattern;
    int position;                    // step index; 0 = step 1 current after load and reset (§4.1)
    vgLib_v2::ResetClockWindow window;   // the 1 ms clock-ignore window (§4.1)
    bool gate_high;                  // `gate` as reported by the last process() call
    struct Out { bool gate; bool struck; bool gate_gap; int position; };
    void load(int window_samples);   // the implicit reset at patch load: step 1, window armed, no strike
    Out process(bool reset_edge, bool clock_edge, int window_samples,
                int hits, int steps, int rot);   // reset FIRST, then clock: the argument order is the processing order
};
```

`process` does, in order: `pattern.compute(hits, steps, rot)` (a no-op unless the
triple changed); on `reset_edge`, `position = 0`, `window.arm(window_samples)`,
`struck = pattern.hit[0]`, `gate_gap = gate_high`; on `clock_edge` with
`!window.open()`, advance (wrapping at `pattern.steps`) and set
`struck = pattern.hit[position]`; `window.tick()`; `gate = pattern.hit[position]`
(false when a shrunken STEPS left the playhead beyond the length). The window length
is whatever the caller passed (the module computes it with
`ResetClockWindow::samplesFor(args.sampleRate)` on the reset sample), so the engine
never sees a sample rate. The pulse shaping stays in the module: it is the only piece
that needs the sample rate beyond the window length.

### 5.2 The module

- Params: `HITS_PARAM, STEPS_PARAM, ROT_PARAM, HITS_ATTEN_PARAM, STEPS_ATTEN_PARAM,
  ROT_ATTEN_PARAM`. Inputs: `CLOCK_INPUT, RESET_INPUT, HITS_CV_INPUT, STEPS_CV_INPUT,
  ROT_CV_INPUT`. Outputs: `TRIG_OUTPUT` only (2026-09-18).
- Two `dsp::SchmittTrigger`s, the engine, a pulse countdown, `trigger_length_index`.
- The context menu carries **Trigger length** (decision 9) and nothing else of its own.

### 5.3 Display snapshot

A snapshot written on the audio thread for the ring: effective hits, steps, rot, the
playhead position, and the 32 hit flags. Plain ints and bools, written only when they
change; the widget reads them in `draw()`. A torn read costs one frame of a stale dot
and nothing else (the VX Drum Sequencer grid and PianoRoll read module state the same
way).

### 5.4 Persistence and identity

- Params are Rack's. The only module JSON is `trigger_length_index` and a `version`
  string. Position is not persisted: it is 0 after every load, with the window armed
  (§4.1 rule 4).
- Nothing is keyed by a name or label. The module slug is the one frozen identifier and
  Rack owns it. No UUIDs are needed because there is no user-created data.
- No migration concerns: a new module, no old patches.

---

## 6. The ring

A port of `panel-ring.js` to a `TransparentWidget` drawing with NanoVG, sized from a
`ring_display` rect on the panel (`PanelHelper::findNamedRect`, the VX Drums
`kit_display` idiom) so the art stays the source of truth for its position.

### 6.1 Drawing

All from `_redraw()` in panel-ring.js:

- a faint guide circle; N dots from 12 o'clock clockwise, `dotR = steps > 16 ? 3.2 : 4.4`
  scaled to the ring's radius;
- hits filled in the accent colour, rests as hollow rings on the well colour;
- a short tick outside the ring at step 0;
- the playhead as a white ring around the current dot;
- centre text `hits/steps` above and `ROT r` below, using `ShareTechMono-Regular.ttf`
  as the other readouts do.
- The ring draws the **effective** pattern from the display snapshot (§5.3, decision 6).
- The plate is dark in both themes (the `_display` rule in the panel generator), so the
  colours are fixed, not theme-mapped.
- Without a module (the browser preview), draw the default 4/16 pattern with the
  playhead on step 1 (PianoRoll idiom).

### 6.2 Interaction

- `onButton` inside the rim hit-test starts a drag; `onDragMove` converts the angular
  delta (`atan2` about the centre, unwrapped to ±π) into whole steps
  (`delta / (2π / steps)`) and writes `ROT_PARAM` through its `ParamQuantity`, wrapped
  `% steps` as the JS does. One `history::ParamChange` per gesture (begin/end values),
  so undo works like a knob.
- Double-click sets ROT to 0.
- **Drag direction.** Dragging clockwise moves the dots clockwise. With
  `pattern[i] = base[(i + rot) % s]`, increasing ROT moves the hits to *earlier*
  indices, which is counter-clockwise on a clockwise ring, so the widget maps a
  clockwise angular delta to a **negative** rotation change (and a counter-clockwise
  one to a positive change). panel-ring.js adds a positive clockwise delta to `rot`
  directly; the port does not copy that. The DSP sign is part of the contract (§3.3)
  and does not change; only the widget's mapping is reversed.
- A right-click on the ring falls through to the module menu (no ring menu).

---

## 7. Panel

Generated, not hand-drawn, by `scripts/generate_euclid_panel.py`, the way
`scripts/generate_vx_drums_panels.py` does it. The parts every generated panel shares
(the Pilat glyph tables and kerning, the house constants, the text layout, the SVG
emitter and the light and dark validators) were lifted out of that single-file script
on 2026-09-16 into the `scripts/panel_gen/` package, with the panel width carried by
the geometry table rather than a module global, so both generators import one copy and
the Euclid script is its geometry table and little else. The output contract is the
same: anchors are `#213d42` circles for jacks and `#d8d6cf` dots for knobs and trimpots, ids suffixed
`_input`, `_output`, `_knob`, plus one `ring_display` rect; `generate_dark_panel.py`
produces the dark file and the display rect is re-inserted after the pass. Both screw
pairs (a standalone module). Front-panel PNGs for the manual are rendered by the
generator's `--render-png`.

Layout, top to bottom, at 9 HP (135 px; columns at cx = 22.5, 67.5, 112.5 px), as of
2026-09-18:

| Row | Contents | Anchor ids |
|---|---|---|
| branding | VOXGLITCH wordmark at its house position, the EUCLID frame to its right at (74.8, 12.85) (`panel["frame_xy"]`, the emitter's one override) | |
| ring | `ring_display` rect, 117 px square at (9, 30) | `ring_display` |
| knobs | HITS, STEPS, ROT at cy 192, each label ABOVE its knob (baseline 170) | `hits_knob`, `steps_knob`, `rot_knob` |
| attenuverters | three `Trimpot`s at cy 232, unlabelled | `hits_atten_knob`, `steps_atten_knob`, `rot_atten_knob` |
| CV jacks | HITS, STEPS, ROT at cy 268, unlabelled; a 1.2 px rule from the knob centre to the jack centre, under the components, ties each column together | `hits_input`, `steps_input`, `rot_input` |
| bottom | CLK, RST (no plate) and TRIG on the outputs plate (94, 295, 37 x 57), all at cy 328, labels above | `clk_input`, `rst_input`, `trig_output` |

The 8 HP panel stacked two rows of two jacks at the bottom (CLK / RST, then GATE /
TRIG on the plate) under an 84 px ring. Dropping GATE and the frame-under-wordmark row,
and widening by one HP, is what let every jack share one row and the ring grow to
117 px. The generator keeps at least 6 px between neighbouring ink at every row
boundary and the plate 13 px above the bottom screws.

---

## 8. Tests (`tests/euclid/`)

Out-of-Rack, compiled directly against the engine header, with the shared tiny
`check.hpp` harness copied from `tests/vx_drum_sequencer/`:

```
g++ -std=c++11 -O2 -I ../../src -o pattern_test     pattern_test.cpp     && ./pattern_test
g++ -std=c++11 -O2 -I ../../src -o engine_test      engine_test.cpp      && ./engine_test
g++ -std=c++11 -O2 -I ../../src -o seq3_parity_test seq3_parity_test.cpp && ./seq3_parity_test
```

(The tests include `modules/Euclid/EuclidEngine.hpp`, and `-I ../../src` also resolves
its include of `vgLib-2.0/sequencer/ResetClockWindow.hpp`. The built binaries are
listed in `tests/euclid/.gitignore`.)

**pattern_test.cpp**: the golden table in §3.3 (including the E(5,8) Bresenham case that
distinguishes this from Bjorklund); rotation wrap (`rot % steps`, rot 31 on 8 steps);
hits > steps clamps to all hits; hits 0 is all rests; steps 1; every (h, s) in 0..32 x
1..32 has exactly `min(h, s)` hits; the CV law's truncation toward zero (1 V -> +3,
-1 V -> -3, 5 V -> +16 at atten 1; anything at atten 0 -> 0; atten -1 inverts).

**engine_test.cpp**, one case per row of the §4.1 table: load leaves step 1 current
with GATE high and no strike; a reset strikes step 1 on that sample and arms the
window; a clock inside the window is dropped; a clock one sample after the window
advances to step 2; a same-sample reset and clock strikes once; a clock followed by a
reset on the next sample strikes N+1 then step 1 (documented, not "fixed"); a reset
while GATE is high reports the one-sample gap; wrap at the effective length; STEPS
shrinking under the playhead reads 0 V then wraps; ROT changed mid-pattern is read on
the next edge; GATE stays high across two consecutive hit steps; `struck` is true only
on the edge or reset sample; the window length is whatever the caller passed, so a
sample-rate change is the caller's business.

**seq3_parity_test.cpp**: the reset rule against Fundamental SEQ3. A float model of
SEQ3's reset/clock logic (`dsp::SchmittTrigger`, `dsp::PulseGenerator`,
`clock && !resetGate`) and `euclid::Sequencer` behind the same Schmitts are driven by
the same xorshift32-generated reset/clock voltages (three seeds x 250 000 samples at
48 kHz: same-sample reset + clock, clock a sample after a reset, reset a sample after a
clock, clocks inside and just outside the window, resets inside a window, bursts, long
idles), with `position == index` checked after every sample; reset-to-clock gaps avoid
the band around SEQ3's 48th sample, its float boundary (measured, SEQ3 closes on our
sample there, but the margin is too thin to promise for every compiler). Then the
Impromptu Clocked defaults (a stopped reset with the clock line held high, no edge at
run start) and a running reset inside the clock's high half, against the expected step
schedule. Last, the exact boundary: `samplesFor` at 48 kHz (48), 44.1 kHz (45), 96 kHz
(96); a clock 47 / 48 samples after a reset (44.1 kHz: 44 / 45) swallowed / accepted by
us and, at those two rates, by SEQ3's float model on the same samples; and 96 kHz,
where SEQ3's float pulse runs 97 samples against our 96, so a clock exactly 96 samples
after a reset is the one sample the two disagree on (95 and 97 agree; §4.1 rule 3).

Compile gate for the Rack TU from WSL (the plugin itself is built on Windows):

```
g++ -std=c++11 -fsyntax-only -DARCH_LIN -DARCH_X64 -O0 \
  -Isrc -I/mnt/c/Code/Rack-SDK/include -I/mnt/c/Code/Rack-SDK/dep/include \
  -Wall -Wextra -Wno-unused-parameter -Wsuggest-override src/modules/Euclid.cpp
```

Then `/vcv-static-analysis` for the cppcheck baseline and `/release-check` before
submitting.

**In-Rack sync check** (the test the drum sequencer complaint would have caught):
Clocked at defaults into Euclid and into VCV SEQ3 with gates on every step, both fed
CLK and RESET from Clocked; stop, reset, run, and confirm both TRIG outputs fire on the
same beats; then press Clocked's reset mid-bar several times and confirm they never
drift apart.

---

## Sources

- VCV Rack manual, Voltage Standards: https://vcvrack.com/manual/VoltageStandards
- Fundamental SEQ3 source: https://github.com/VCVRack/Fundamental/blob/v2/src/SEQ3.cpp
- Impromptu Modular README (1ms-clock-ignore-on-reset, gate retriggering, Clocked's reset behaviour): https://github.com/MarcBoule/ImpromptuModular
- Impromptu sequencer reference code: https://marcboule.github.io/ImpromptuModular/README_SeqCode.html
- "Reset Sequencers are one step off", VCV Community: https://community.vcvrack.com/t/reset-sequencers-are-one-step-off/12898
- "Seq3: Resetting with clock question", VCV Community: https://community.vcvrack.com/t/seq3-resetting-with-clock-question/10860
- `~/vxsynth/app/studio/lib/audio/wasm/modules/euclid.c` (the behaviour contract), `.../modules/definitions/euclid.js` and `.../canvas/floating-panel/panel-ring.js` (the panel and the ring), `~/vxsynth/public/studio/docs/euclid.md` (the manual seed), all as of 2026-09-16
- `/mnt/c/Code/voxglitch_devices/src/GrooveboxAdvanced/modules/GAEuclid.hpp` (the origin, NanoVG reference for the ring)
- `docs/implementation_plans/euclid-migration.md`, the proposal this record binds
- `docs/modules/vx-drum-sequencer/rack-port-design.md`, the house conventions this port follows and the reset rule it does not
