# Euclid — migrating the vxsynth Euclidean Sequencer to Voxglitch

A proposal for bringing the vxsynth **Euclidean Sequencer** (`Euclid Seq`) into this
plugin as a standalone Rack module. Nothing here is decided; §3 lists the calls that
are the owner's, with a recommendation for each. Once those are made, this document
should be replaced by a `rack-port-design.md` in `docs/modules/<slug>/`, in the style
of `docs/modules/vx-drum-sequencer/rack-port-design.md`.

Source citations are to `~/vxsynth` as of 2026-09-16. §4.1 was rewritten on the same
day after checking the VCV standard: the reset rule it now proposes is **not** the one
the VX Drums port used, and §4.6 explains why that matters beyond this module.

---

## 1. Where the code actually comes from

The vxsynth module is itself a port. Its DSP header says so: *"Ported faithfully from
Groovebox Advanced GAEuclid (GAEuclidDSP)"*. There are three copies of the same
algorithm, and the oldest one is already Rack C++:

| Copy | Where | Notes |
|---|---|---|
| **GAEuclid** (origin) | `/mnt/c/Code/voxglitch_devices/src/GrooveboxAdvanced/modules/GAEuclid.hpp` | An internal module of Groovebox Advanced in the *Voxglitch Devices* plugin. Rack C++, NanoVG ring overlay with a Toussaint polygon. No attenuverters. Not a standalone module |
| **euclid.c** | `~/vxsynth/app/studio/lib/audio/wasm/modules/euclid.c` | The WASM port. Byte-for-byte the same Bresenham pattern and step logic; **adds three bipolar attenuverters** on the CV jacks |
| **euclid.js / panel-ring.js** | `~/vxsynth/app/studio/lib/modules/definitions/euclid.js`, `.../canvas/floating-panel/panel-ring.js` | The panel definition and the Ring widget (rim-drag rotation, double-click reset, live playhead) |
| manual | `~/vxsynth/public/studio/docs/euclid.md` | Reusable as the seed for the Rack user manual |

So the migration is not "translate C to C++". It is: write a new standalone Voxglitch
module whose **behaviour contract is euclid.c** (the vxsynth feature set, attenuverters
included), whose **display is panel-ring.js**, and whose Rack idioms (thresholds, pulse
lengths, panels, persistence, tests) follow the house conventions set by the VX Drums
port, **except reset**, which follows the VCV standard (§4.1). GAEuclid.hpp is useful
as a NanoVG reference for the ring, nothing more.

Note for anyone tempted to "correct" the algorithm: the source uses **Bresenham**, not
Bjorklund. They produce the same rhythms up to rotation, but not the same rotation.
E(5,8) is `x.x.xx.x` here, where Bjorklund gives `x.xx.xx.`. The Bresenham output is
the contract (vxsynth patches and the GA module depend on it); the tests in §9 pin it.

---

## 2. The behaviour contract (carried over unchanged)

Everything in this section is copied from `euclid.c` as is.

**Parameters**

| Param | Range | Default | Notes |
|---|---|---|---|
| HITS | 0..32, integer | 4 | clamped to STEPS at use |
| STEPS | 1..32, integer | 16 | |
| ROT | 0..31, integer | 0 | applied `% steps` |
| HITS CV atten | -1..1 | 0 | bipolar; 0 = jack ignored |
| STEPS CV atten | -1..1 | 0 | |
| ROT CV atten | -1..1 | 0 | |

**Jacks**: CLK, RST, HITS CV, STEPS CV, ROT CV in; GATE, TRIG out. Mono only.

**CV law**: `effective = clamp(knob + (int)(cv * 3.2 * atten), lo, hi)`. The `(int)`
truncates toward zero, so ±5 V is ±16 steps and 1 V is 3 steps, not 4. Hits are then
clamped to the effective step count; rotation to `0..steps-1`.

**Pattern** (`euclid_recompute_pattern`, dirty-cached on the three effective ints):

```
base[0] = 1
base[i] = floor(i*h/s) > floor((i-1)*h/s)      for i in 1..s-1   (integer division)
pattern[i] = base[(i + rot) % s]
h >= s  -> all hits;   h == 0 -> all rests
```

Golden values for the tests (§9), computed from that rule:

| (h, s) | pattern |
|---|---|
| (3, 8) | `x..x..x.` |
| (5, 8) | `x.x.xx.x` |
| (4, 16) | `x...x...x...x...` |
| (5, 16) | `x...x..x..x..x..` |
| (3, 8) rot 1 | `..x..x.x` (increasing ROT pulls hits to *earlier* indices) |

**Outputs**: GATE is 10 V for the whole of any hit step (consecutive hit steps make one
continuous gate, no re-edge). TRIG is a 10 V pulse raised when the playhead lands on a
hit. Both are 0 V on rests.

**Playhead**: advances one step per clock rising edge, modulo the effective step count,
so a STEPS change that shrinks below the playhead wraps on the next clock. Step 1
(index 0) is the current step after load and after reset, and GATE reflects it, so
GATE is high at rest whenever HITS > 0. That is the source's behaviour and, as §4.1
shows, the standard's.

---

## 3. Decisions for the owner

| # | Decision | Recommendation |
|---|---|---|
| 1 | **Name and slug.** The slug is Rack's identity for the module and can never change once a version ships. "Euclid" is a common module name in the library | Module name **Euclid**, slug `euclid`, `panelName` on the frame **EUCLID** (matches the vxsynth label). If a more distinctive house name is wanted (One Point / One Zero / Hazumi style), decide it now; it only costs a rename before the first release |
| 2 | **Width** | **8 HP** (120 px). Three columns of knobs, trimpots and CV jacks at a 40 px pitch, a four-jack row at the bottom, and an 84 px ring. 10 HP if the ring should keep vxsynth's 112 px size |
| 3 | **Keep the attenuverters** (vxsynth) or bare CV jacks (GAEuclid)? | Keep them. They are the one thing vxsynth added, and Rack users expect a CV jack next to a knob to have a scaler |
| 4 | **Attenuverter default: 0** (vxsynth: jack silent until turned up) **or +1** (CV live when patched)? | 0, for parity with vxsynth patches and the manual text. Flag it in the manual, since a patched-but-silent jack is a classic support question |
| 5 | **Ring is a control** (rim drag rotates, double-click zeroes, as vxsynth) or a display only? | A control. It is the module's signature and it is cheap: one param written through `ParamQuantity` with a history entry per drag |
| 6 | **Ring draws the effective (CV-modulated) pattern** (GAEuclid) **or the knob pattern with only the playhead live** (vxsynth)? | Effective. In Rack you can see nothing else about the CV, and GAEuclid already did it. The knobs still show knob positions |
| 7 | **Toussaint polygon** joining the hits (GAEuclid) | Leave it out at first; the vxsynth look is the target. Trivial to add later |
| 8 | **Reset convention** | **The VCV standard** (§4.1): reset makes step 1 current and plays it, clock edges are ignored for 1 ms after a reset, the next clock plays step 2. **Not** the VX Drum Sequencer's `position = -1` rule, which is the "offset" family that the sync complaint is about (§4.6) |
| 9 | **Trigger length** | 1 ms default, context-menu list `{1, 2, 5, 10, 20, 50, 100, 200} ms` as ArpSeq and VX Drum Sequencer, persisted as an index |
| 10 | **Rack-free engine + out-of-Rack tests**, as VX Drum Sequencer | Yes. The engine is ~80 lines; the tests are what pin Bresenham and the reset cases |
| 11 | **Fix VX Drum Sequencer's reset first** (§4.6), so Euclid inherits a tested reset idiom, or ship Euclid on the standard and fix the drum sequencer after? | Fix the drum sequencer first. It is the module with the complaint, its engine already has tests, and the same helper then serves both |

---

## 4. Where the port diverges from euclid.c, and why

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

**The rule for this module**, which is SEQ3's:

1. **RST rising edge** (Schmitt, `constants::gate_low_trigger` / `gate_high_trigger`,
   0.1 V / 2.0 V): `position = 0`. Step 1 is current: GATE follows `pattern[0]`, and
   if step 1 is a hit **TRIG fires now**. If GATE was already high, it is forced to 0 V
   for one sample so a downstream envelope sees a fresh edge (Impromptu's gate
   retrigger, at the drum sequencer's one-sample width instead of 1 ms). A **1 ms
   clock-ignore window** is armed, `ceil(0.001 * args.sampleRate)` samples, computed
   when armed, never cached.
2. **CLK rising edge** inside the window: ignored. Outside it: advance-then-play,
   `position = (position + 1) % steps`, TRIG if a hit, GATE from the pattern.
3. **Order inside one sample:** RST, then CLK, then outputs. A same-sample reset and
   clock plays step 1 once, at the reset; the clock is inside the window.
4. **Patch load** is an implicit reset (Impromptu): `position = 0`, GATE from the
   pattern, window armed, **no TRIG**.
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
| Patch load | Step 1 current, GATE shows it, no TRIG, window armed |
| STEPS shrinks below the playhead | GATE reads a cleared slot (0 V) until the next clock wraps to step 1 |

### 4.2 Trigger pulse

The source fires a fixed **48 samples** (1 ms only at 48 kHz). The port computes the
pulse from `args.sampleRate` and the menu length (decision 9). `dsp::PulseGenerator`
only extends a pulse that is already high, so a clock faster than the pulse length
would merge consecutive hits; use the VX Drum Sequencer rule instead (a sample
countdown, with one forced 0 V sample when a new hit lands while TRIG is still high).
Single lane, so it is a dozen lines, not the whole `TriggerShaper`. The same one-sample
gap serves the GATE retrigger in §4.1.

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
  module directly (§6).
- The vxsynth panel's "Attenuverters" group frame is a panel-art matter, not code.

### 4.6 What this means for the sequencers already shipped

This is outside the Euclid port, but the research above explains a live complaint,
so it is recorded here until it has a ticket of its own.

- **VX Drum Sequencer** is in the *offset* family by design
  (`rack-port-design.md` §1 "Reset", §4.2): `position = -1` after a reset, nothing
  fires at the reset instant, no clock-ignore window, "the next clock plays step 1".
  That is exactly the family the thread found one step behind. With Clocked at its
  defaults it is one step behind after every stop-reset-run (no edge at run start,
  so its step 1 waits for beat 2) and after every running reset that lands while the
  clock output is high. That is the reported "does not sync" behaviour. The port's
  design record reasoned from the source's 1 ms window swallowing a coincident clock
  and concluded "no window, ever"; the standard's answer to the same problem is the
  opposite one: keep the window, and make the reset itself play step 1.
  Fix sketch, in the engine: `Sequencer::rewind()` becomes a reset that sets
  `position = 0`, fires step 1 through `fire()` (ratchets arm if a period is known),
  and arms a 1 ms window; `process()` drops clock edges while the window counts down.
  The chain **hand-off** must keep its current "-1 then advance" shape: the next
  member's step 1 is played by the next clock edge, not by the hand-off. The existing
  `engine_test.cpp` and `chain_test.cpp` cases for reset need to be rewritten to the
  standard, and the case table in `rack-port-design.md` §4.3 replaced by the one
  above.
- **ArpSeq** is advance-then-play from -1 with the window removed in #278
  (commit 30dcc68). Same family. Its `legacy_reset_mode` is the nearer thing to the
  standard, but it does not play step 1 at the reset.
- **Digital Sequencer / XP** offer "Reset Mode: Next clock input" (default, offset
  family) and "Instant" (standard), and arm a **10 ms** window in both, ten times the
  standard. GlitchSequencer and AutobreakStudio carry the same window.

Recommendation: one ticket, "align the sequencers on the VCV reset standard", with a
shared helper (§5.1) and a legacy option only where a shipped patch could depend on
the old timing. Decision 11 asks whether that ticket goes before or after Euclid.

---

## 5. Architecture

Mirror VX Drum Sequencer: a Rack-free engine, a thin Rack adapter, a panel widget, and
one custom display widget.

```
src/modules/Euclid.cpp                      the TU: includes, using namespace, createModel
src/modules/Euclid/EuclidEngine.hpp         Rack-free: pattern(), Playhead, Sequencer::process
src/modules/Euclid/Euclid.hpp               Module: params/inputs/outputs, Schmitts, pulse, JSON
src/modules/Euclid/EuclidRingWidget.hpp     the ring display + rotation control
src/modules/Euclid/EuclidWidget.hpp         ModuleWidget: PanelHelper anchors, menu
src/plugin.hpp                              extern Model* modelEuclid;
src/plugin.cpp                              p->addModel(modelEuclid);
plugin.json                                 slug, name, description, tags ["Sequencer"], manualUrl
res/modules/euclid/euclid_panel.svg         generated (§7)
res/modules/euclid/euclid_panel-dark.svg    generated
scripts/generate_euclid_panel.py            the generator
docs/modules/euclid/user_manual.md          seeded from vxsynth's euclid.md
docs/modules/euclid/rack-port-design.md     the bound version of this document
tests/euclid/{README.md, check.hpp, pattern_test.cpp, engine_test.cpp}
README.md                                   module list entry
```

`Makefile` needs nothing: `src/modules/*.cpp` is globbed.

### 5.1 The engine (`namespace euclid`)

```cpp
static const int MAX_STEPS = 32;

struct Pattern { bool hit[MAX_STEPS]; int hits, steps, rot; };   // effective ints + result
void   computePattern(int hits, int steps, int rot, Pattern& out); // §2, Bresenham
int    effectiveValue(int knob, float cv, float atten, int lo, int hi); // §2 CV law

struct Playhead {
    int position = 0;                // step 1 is current after load and reset (§4.1)
    int advance(int steps);          // (position + 1) % steps
    void reset() { position = 0; }
};

struct Sequencer {
    Pattern pattern;                 // dirty-cached on (hits, steps, rot)
    Playhead playhead;
    int ignore_clock = 0;            // samples left in the 1 ms window
    struct Out { bool gate; bool struck; bool gate_gap; int position; };
    Out process(bool clockEdge, bool resetEdge, int windowSamples,
                int hits, int steps, int rot);
};
```

`process` does, in order: on `resetEdge`, `playhead.reset()`, `ignore_clock =
windowSamples`, `struck = pattern.hit[0]`, `gate_gap = (gate was high)`; recompute the
pattern if the ints changed; on `clockEdge` with `ignore_clock == 0`, advance and set
`struck = pattern.hit[position]`; decrement `ignore_clock`; `gate = pattern.hit[position]`.
The pulse shaping stays in the module (it is the only piece that needs the sample rate
beyond the window length the module passes in), so the engine is pure integers and
compiles standalone for the tests.

If decision 11 is "drum sequencer first", the reset-and-window part of this (two
Schmitts, the window countdown, the RST-before-CLK order) should become a small shared
header, `src/vgLib-2.0/sequencer/ResetClock.hpp` or similar, that both engines call,
so the standard is written once.

### 5.2 The module

- Params: `HITS_PARAM, STEPS_PARAM, ROT_PARAM, HITS_ATTEN_PARAM, STEPS_ATTEN_PARAM,
  ROT_ATTEN_PARAM`. Inputs: `CLOCK_INPUT, RESET_INPUT, HITS_CV_INPUT, STEPS_CV_INPUT,
  ROT_CV_INPUT`. Outputs: `GATE_OUTPUT, TRIG_OUTPUT`.
- Two `dsp::SchmittTrigger`s, the engine, a pulse countdown, `trigger_length_index`.
- A **display snapshot** written on the audio thread for the ring: effective hits, steps,
  rot, the playhead position, and the 32 hit flags. Plain ints and bools, written only
  when they change; the widget reads them in `draw()`. A torn read costs one frame of
  a stale dot and nothing else (the VX Drum Sequencer grid and PianoRoll read module
  state the same way).
- JSON: `trigger_length_index` and a `version` string. Params persist through Rack.

---

## 6. The ring widget

A port of `panel-ring.js` to a `TransparentWidget` drawing with NanoVG, sized from a
`ring_display` rect on the panel (`PanelHelper::findNamedRect`, the VX Drums
`kit_display` idiom) so the art stays the source of truth for its position.

**Drawing** (all from `_redraw()` in panel-ring.js):

- a faint guide circle; N dots from 12 o'clock clockwise, `dotR = steps > 16 ? 3.2 : 4.4`
  scaled to the ring's radius;
- hits filled in the accent colour, rests as hollow rings on the well colour;
- a short tick outside the ring at step 0;
- the playhead as a white ring around the current dot;
- centre text `hits/steps` above and `ROT r` below, using `ShareTechMono-Regular.ttf`
  as the other readouts do.
- The plate is dark in both themes (the `_display` rule in the panel generator), so the
  colours are fixed, not theme-mapped.
- Without a module (the browser preview), draw the default 4/16 pattern with the
  playhead on step 1 (PianoRoll idiom).

**Interaction** (decision 5):

- `onButton` inside the rim hit-test starts a drag; `onDragMove` converts the angular
  delta (`atan2` about the centre, unwrapped to ±π) into whole steps
  (`delta / (2π / steps)`) and writes `ROT_PARAM` through its `ParamQuantity`, wrapped
  `% steps` as the JS does. One `history::ParamChange` per gesture (begin/end values),
  so undo works like a knob.
- Double-click sets ROT to 0.
- **Check the drag direction before copying it.** With `pattern[i] = base[(i+rot)%s]`,
  increasing ROT moves the hits to earlier indices, which is counter-clockwise on a
  clockwise ring, while panel-ring.js adds a positive (clockwise) angular delta to
  `rot`. Whether that feels backwards in vxsynth should be confirmed in the app; in the
  port, map the drag so that dragging clockwise moves the dots clockwise. The DSP sign
  is part of the contract and must not change.
- A right-click on the ring should fall through to the module menu (no ring menu).

---

## 7. Panel

Generated, not hand-drawn, the way `scripts/generate_vx_drums_panels.py` does it. That
script is a single 1060-line file with the Pilat glyph outlines, kerning tables,
validation and dark-pass embedded. Two options:

1. **Lift the shared parts** (`text_paths`, the glyph tables, `emit_svg`, `validate_light`,
   `validate_dark`, `run_dark_generator`, `reinsert_display_rects`) into
   `scripts/panel_gen/` and have both scripts import them. Right thing, one-time cost.
2. Copy the file and replace the two geometry tables. Fast, duplicates ~800 lines.

Recommend 1. Either way the output contract is the same: anchors are `#213d42` circles
for jacks and `#d8d6cf` dots for knobs and trimpots, ids suffixed `_input`, `_output`,
`_knob`, plus one `ring_display` rect; `generate_dark_panel.py` produces the dark file and
the display rect is re-inserted after the pass. Both screw pairs (a standalone module).

Proposed layout, top to bottom, at 8 HP (columns at x = 20, 60, 100 px):

| Row | Contents | Anchor ids |
|---|---|---|
| name frame | EUCLID (house frame, Pilat Condensed) | |
| ring | `ring_display` rect, ~84 px square, centred | `ring_display` |
| knobs | HITS, STEPS, ROT | `hits_knob`, `steps_knob`, `rot_knob` |
| attenuverters | three `Trimpot`s, labelled as one "CV" group | `hits_atten_knob`, `steps_atten_knob`, `rot_atten_knob` |
| CV jacks | HITS, STEPS, ROT | `hits_input`, `steps_input`, `rot_input` |
| bottom | CLK, RST, then GATE, TRIG on the outputs plate | `clk_input`, `rst_input`, `gate_output`, `trig_output` |

The four-jack bottom row at 30 px pitch is the tight spot; if it does not fit the house
jack size with labels, that is the argument for 10 HP (decision 2).

---

## 8. Persistence and identity

- Params are Rack's. The only module JSON is `trigger_length_index` and `version`.
- Nothing is keyed by a name or label. The module slug is the one frozen identifier and
  Rack owns it. No UUIDs are needed because there is no user-created data.
- No migration concerns: a new module, no old patches.

---

## 9. Tests (`tests/euclid/`)

Out-of-Rack, compiled directly against the engine header, with the shared tiny
`check.hpp` harness copied from `tests/vx_drum_sequencer/`:

```
g++ -std=c++11 -O2 -I ../../src/modules/Euclid -o pattern_test pattern_test.cpp && ./pattern_test
g++ -std=c++11 -O2 -I ../../src/modules/Euclid -o engine_test  engine_test.cpp  && ./engine_test
```

**pattern_test.cpp**: the golden table in §2 (including the E(5,8) Bresenham case that
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

## 10. Documentation

- `docs/modules/euclid/user_manual.md`, seeded from `~/vxsynth/public/studio/docs/euclid.md`
  (Overview, Controls, Inputs & outputs, Tips are all directly reusable), with Rack
  specifics added: volts, the attenuverter default (decision 4), the trigger-length menu,
  and a reset paragraph: *reset puts Euclid on step 1 and plays it; the next clock is
  step 2; clocks arriving within 1 ms of a reset are ignored, as the VCV standard asks,
  so Euclid stays in step with SEQ3, Impromptu Clocked and the other sequencers that
  follow it.* Front-panel PNGs rendered by the generator's `--render-png`.
- `docs/modules/euclid/rack-port-design.md`: this document, rewritten as bound decisions
  once §3 is settled.
- A release-notes entry and a README line.
- `plugin.json` description, in the house style: one sentence of what it is, one of what
  it has.

---

## 11. Work plan

Each step compiles and is reviewable on its own.

1. **Settle §3.** Name, slug, width, the yes/no calls, and decision 11 (drum sequencer
   first or not). Half an hour.
2. **Engine + tests.** `EuclidEngine.hpp`, `tests/euclid/`, the §4.1 case table as
   tests. If decision 11 is "drum sequencer first", the shared reset helper comes out
   of that work and this step consumes it.
3. **Module + widget skeleton** with a placeholder panel (a copy of the One Zero SVG
   with the anchors renamed is enough to compile and place jacks). Syntax-check from WSL.
4. **Panel generator** (§7), light and dark SVGs, PNG renders.
5. **Ring widget** (§6), including the drag-direction check.
6. **Manual, design record, plugin.json, README, release notes** (§10).
7. **Gates**: `/vcv-static-analysis`, `/release-check`; Windows build; the in-Rack sync
   check from §9, plus CV into every jack and a STEPS sweep.

Steps 2 and 4 are independent and can be done in parallel.

---

## 12. Open questions

- [ ] Name and slug (frozen forever once shipped).
- [ ] 8 HP or 10 HP.
- [ ] Attenuverter default 0 or +1.
- [ ] Ring is a control (drag/double-click) or a display.
- [ ] Ring shows the effective pattern or the knob pattern.
- [ ] Toussaint polygon: no / later / now.
- [ ] Confirm in vxsynth which way the rim drag actually moves the dots.
- [ ] Panel generator: shared `scripts/panel_gen/` refactor, or a copy.
- [ ] Fix VX Drum Sequencer's reset to the standard before or after Euclid (§4.6).

---

## Sources

- VCV Rack manual, Voltage Standards: https://vcvrack.com/manual/VoltageStandards
- Fundamental SEQ3 source: https://github.com/VCVRack/Fundamental/blob/v2/src/SEQ3.cpp
- Impromptu Modular README (1ms-clock-ignore-on-reset, gate retriggering, Clocked's reset behaviour): https://github.com/MarcBoule/ImpromptuModular
- Impromptu sequencer reference code: https://marcboule.github.io/ImpromptuModular/README_SeqCode.html
- "Reset Sequencers are one step off", VCV Community: https://community.vcvrack.com/t/reset-sequencers-are-one-step-off/12898
- "Seq3: Resetting with clock question", VCV Community: https://community.vcvrack.com/t/seq3-resetting-with-clock-question/10860
