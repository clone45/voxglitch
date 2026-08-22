# Piano Roll — VCV Rack Port Design

Where the Rack version deliberately diverges from the vxsynth original, and why.
Read alongside the four vxsynth documents in this directory, which describe the
source behavior in detail.

**Status:** design resolved. Every claim below was investigated against the real
Rack SDK headers (`/mnt/c/Code/Rack-SDK`), the Voxglitch source, and the live
vxsynth source (`/home/clone45/vxsynth`), then adversarially re-verified by a
second reader who re-opened every cited file. Findings that survived verification
are stated plainly; the places where the first reading was **wrong** are marked
⚠ CORRECTED, because several would have shipped as bugs.

---

## 1. Decisions locked

| Decision | Value |
|---|---|
| Tracks | **8** (was 4) |
| Per-track output | **Polyphonic** V/OCT + GATE pair |
| Polyphony sizing | **Auto from content** — channels = max simultaneous notes, clamped to 16 |
| Clock | **External only**, and **1 pulse = 1 step (a 16th)**, always — see §6 |
| Recording input | **Polyphonic** V/OCT + GATE pair |
| Panel width | **52 HP** = exactly **780 px** (`RACK_GRID_WIDTH` = 15, `common.hpp:18`) |
| Panel height | **380 px** (`RACK_GRID_HEIGHT`) |
| Jack placement | Inputs **left** of the editor, outputs **right** |
| Grid row height | **9 px** — 35 rows, 2.9 octaves (§7) |
| Undo | **Rack's history system**, custom `history::ModuleAction` (§10) |
| Note identity | **Opaque 53-bit random id** per note, minted once (§9) |

---

## 2. Dropped from vxsynth

- **Song clock mode** and the entire derived-position branch.
- **Internal clock**: `bpm`, the sample accumulator, `run`, `stepNow`, Stop's gate release.
- **The transport strip** (⏮ ⏹ ▶) and the **BPM value box**.
- **`clockmode`** entirely, and every mode-change silencing rule.
- **The rewind action** as a live store-bypassing param.
- **The live-param mechanism** — Rack widgets share memory with the module.

The control bar collapses from six controls to three: **Snap · Track · REC**.

---

## 3. What survives, and gets more important

**Clock-period measurement.** With Int and Song gone this is the **only** source of
step timing, and it feeds the record quantizer, which writes note *lengths* into
persisted data. A mis-measured period corrupts recorded material permanently.

⚠ **CORRECTED — two things the first reading got wrong here:**

1. **Do *not* zero the measured period on reset.** `pr_reset()` (pianoroll.c:227-237)
   deliberately does *not* touch `stepSamples` or `extClkCounter`; the only places
   `stepSamples = 0` appears are init (:270) and a clock-mode change (:320, *"Ext
   must re-measure"*). Zeroing it on reset would give the first note recorded after
   every reset a garbage length. Reset the *anchor*, never the period.
2. **There is a real bug in the vxsynth source to fix in the port.** `extClkCounter++`
   runs unconditionally at :412, but the counter is only zeroed at :421 — *inside*
   the accept branch. An edge arriving during the post-reset ignore window is
   rejected without zeroing, so the next accepted edge measures **two** intervals
   and doubles `stepSamples`. The correct rule is **zero the sample counter whenever
   you reject an edge**, not just when you accept one.

---

## 4. The polyphony model

vxsynth is **mono per track, last-note-wins**. Under polyphony, overlapping notes
become chords — the main reason to reach for a piano roll.

### 4.1 Channel count is derived from content — circularly

⚠ **CORRECTED.** The obvious linear sweep is **wrong**, because a note sustains
straight through the loop wrap: `pr_step()` wraps position with a bare
`s->position = 0` and never touches any gate, and aging is a pure countdown
independent of position. A note at step 60, length 8, loop 64 sounds at steps
60,61,62,63,**0,1,2,3**. A linear sweep under-counts channels — measured wrong in
~22% of randomised patterns — and under-counting means **notes are silently
dropped**.

```cpp
// Channels needed on `track`. Returns 1..PORT_MAX_CHANNELS.
int PianoRoll::channelsForTrack(int track) const
{
    const int L = std::max(1, loopSteps);
    std::vector<int> delta(L + 1, 0);

    for (const Note& n : notes) {
        if (n.track != track || n.length <= 0) continue;
        if (n.start < 0 || n.start >= L) continue;   // never plays — see §4.4

        const int len = std::min(n.length, L);       // self-retrigger clamp
        const int s = n.start;
        const int e = s + len;                       // half-open [s, e)

        if (e <= L) { delta[s]++; delta[e]--; }
        else        { delta[s]++; delta[0]++; delta[e - L]--; }   // head + wrapped tail
    }

    int running = 0, peak = 0;
    for (int k = 0; k < L; k++) { running += delta[k]; peak = std::max(peak, running); }
    return rack::math::clamp(peak, 1, rack::engine::PORT_MAX_CHANNELS);
}
```

Every clause is load-bearing:

- **Half-open `[s, e)`** mirrors `pr_step`'s age-then-start order: a note ending at
  step *k* and one starting at *k* are never simultaneous.
- **`min(n.length, L)`** encodes the self-retrigger (§4.4).
- **`start >= L` skip** — those notes provably never play.
- **`clamp(…, 1, 16)` is mandatory, not defensive** — see §4.2.

### 4.2 ⚠ The clamp is a memory-safety requirement

`Port::setVoltage()` (Port.hpp:43) and `setChannels()` (Port.hpp:156) have **zero
bounds checking**, and `voltages[16]` aliases the port's own `channels` byte. Two
distinct failure modes:

- An unclamped count > 16 leads to out-of-bounds writes — not from `setChannels`
  itself, which merely stores a bad count, but from every subsequent
  `setVoltage(v, 16…)`, and from Port's own `c < channels` loops
  (`clearVoltages`, `readVoltages`, `writeVoltages`, `getVoltageSum`).
- `setChannels` takes **`uint8_t`**, so an unclamped `int` of 256 truncates to 0,
  which the function then converts to 1 with all voltages cleared.

A user stacking 17 notes on one step of one track currently corrupts memory.

### 4.3 ⚠ Publish channel counts every sample, not on edit

`setChannels()` **silently no-ops on a disconnected port** (Port.hpp:157-160:
`if (this->channels == 0) return;`). A "compute on edit, push once" design
therefore loses the count for any output with no cable yet, and the track comes up
**mono** the first time the user patches it.

So: the editor computes the count into `trackChannels[t]`; `process()` publishes it
unconditionally every sample, before writing voltages:

```cpp
uint8_t n = trackChannels[t];                 // derived, clamped, >= 1
outputs[VOCT_OUTPUT + t].setChannels(n);
outputs[GATE_OUTPUT + t].setChannels(n);
for (uint8_t c = 0; c < n; c++) { … }
```

The cost is one branch and a zero-iteration loop. **Never call `setChannels()` from
the UI thread.**

### 4.4 Runtime rules

- **Allocate first-free at note start and hold the channel for the note's whole
  duration**, including across the loop wrap. Do not reallocate at the wrap.
- **Age all channels before starting any note**, mirroring `pr_step`.
- ⚠ **Carry the retrigger gap over to channel reuse.** The ~1 ms gate-low gap must
  fire when a *channel* is reused, or a back-to-back note merges into one long gate
  for a downstream ADSR — exactly what the age-then-start order exists to prevent.
- **Quiesce on shrink.** Shrinking zeroes the dropped channels (Port.hpp:162-164),
  hard-cutting a held gate while V/OCT snaps to 0 V (C4) — an audible glide on any
  envelope with a release tail. Follow Rack's own precedent (`MidiParser::setChannels`
  calls `panic()` on any change): on a **shrink**, force that track's gates low for
  one sample and re-latch. Growth is glitch-free and needs no quiesce.
- **A note longer than the loop is re-articulated once per pass**, not held forever
  — at the self-match the retrigger fires, dropping the gate ~1 ms each pass.
  (Measured: exactly one retrigger per pass.)

### 4.5 ⚠ The voicing model must be chosen before trusting the clamp

`min(len, loop)` is correct **only** for vxsynth's self-reset semantics. A third,
equally literal model — let an over-long note ring its full declared length and
allocate a *new* voice each time its start comes round — needs `ceil(len/loop)`
channels (loop 64: len 100 → 2, len 200 → 4). Pick the model deliberately; the
counting algorithm follows from it, not the other way round.

**Recommended:** keep vxsynth's self-reset semantics. It is what the source does,
it bounds channels at the loop length, and the once-per-pass retrigger is musically
sensible for a looping sequencer.

---

## 5. Polyphonic recording

- **Per-channel Schmitt triggers** on REC GATE, sized to `getChannels()` (0 when
  unpatched).
- Read pitch with **`getPolyVoltage(c)`** — its mono-normals-to-all-channels
  semantics mean a mono pitch source paired with a poly gate source just works.
- Per-channel capture state; each channel's gate rise/fall produces its own note,
  all landing on the **active track**.
- **The pitch-change-splits-the-note rule now applies per channel.**
- Everything else carries over: hard 16th quantization, nearest-boundary start snap
  with forward rounding, length from held time with a one-step minimum.

---

## 6. Clock and transport

- **CLK**: one pulse = one 16th step. **RST**: rewind, with a post-reset clock-ignore.
- Use the Voxglitch house idiom: `dsp::SchmittTrigger` with explicit
  `constants::gate_low_trigger` (0.1 V) / `constants::gate_high_trigger` (2.0 V) —
  57 of 60 clocked call sites in the repo do this. Not the SDK defaults (0.0/1.0).
- Use the **`dsp::TTimer<double>` 1 ms** ignore window (OnePoint's form), not the
  older `sampleRate/100` counter — that one is **10 ms** despite its comment saying
  1 ms, which at 300 BPM is 20% of a step.
- On reset do all four: `clock_trigger.reset()` (so a still-high clock line cannot
  fire a phantom step when the window expires), start the timer, set `position = -1`,
  and zero the *sample counter* — but **not** the measured period (§3).
- Measure the period in **samples**, porting `extClkCounter` with its
  `> 1 && < sampleRate * 4.0` debounce window.

### 6.1 ⚠ No clock division or multiplication — decided against

The initial recommendation was to add a clock-rate setting. **Rejected on
verification**, for a reason that only surfaces on close reading:

> **Any rate other than 1:1 requires predicting note onsets.** With a slower clock
> (1 pulse = a quarter note), three of every four 16th steps have no edge to fire
> on and must be interpolated from the previous interval. For an arpeggiator that
> only shifts a gate length; for a piano roll it *places note onsets*, so tempo
> drift is directly audible as rushed or dragged 16ths — and every `step_samples`
> the record quantizer uses becomes a prediction too.

Voxglitch already ships two modules that require a fast clock and simply document
it: GrooveBox (*"The clock needs to be fast — x32 times… use Clocked and set RATIO
to x32"*) and Autobreak (*"Feed it 16th notes for best results"*). The Piano Roll
follows that precedent.

**Mitigation for the silent-failure risk:** surface the measured step period or an
inferred BPM in the UI, so a mis-rated clock is visible rather than silently
corrupting recorded note lengths.

---

## 7. Panel geometry

52 HP = **exactly 780 px** (52 × `RACK_GRID_WIDTH` 15; the mm route agrees:
264.16 mm × 75/25.4 = 780.000). Panel height 380 px. Every Voxglitch port SVG is
23.528 × 23.528 px (radius 11.764).

### 7.1 ⚠ Row height: 9 px, not 8 — the conflict resolved

Two investigations disagreed. The geometry reading recommended 8 px on the grounds
that 9 px "cannot reach three octaves in a 380 px panel." **That is false**, and the
verifier proved it: the editor column (x 54–663) is **x-disjoint from all four screw
boxes** (screws occupy x 15–30 and 750–765 only), so the editor may span nearly the
full panel height. The house precedent already does this — CueResearch places a
child widget at y = 25.

The palette investigation independently argued *against* 8 px: it shrinks the note
body from 7.5 px to 6.5 px, a 13% loss of coloured area, degrading every colour-
separation figure in §8 — and that palette is already at the edge of what the dimmed
colour-vision-deficient case supports.

**Resolution: 9 px rows, with the editor spanning nearly the full panel height.**
Both concerns are satisfied. vxsynth's real baseline is **36 rows / 3.00 octaves**
— *not* the 41 rows an earlier draft of this document claimed — and 9 px reaches
36 rows here too, until the control bar takes a row back (§7.3): the shipped figure
is **35 rows, 2.9 octaves**, one row under the original.

### 7.2 Exact divisibility is required, not cosmetic

`_pitchAt(ly)` has **no clamp to the row count** — every call site clamps only to
0–127. A grid height that is not an exact multiple of `ROW_H` leaves a dead band
that still reports `zone === 'grid'`, so a click there returns a pitch *below* the
last drawn row and the resulting note draws clipped into the band. The grid height
must divide by 9 exactly.

### 7.3 The layout

```cpp
// Panel: 780 x 380 px
static constexpr float CONTROL_BAR_X = 54.0f, CONTROL_BAR_Y =  8.0f;
static constexpr float CONTROL_BAR_W = 609.0f, CONTROL_BAR_H = 32.0f;

static constexpr float EDITOR_X = 54.0f,  EDITOR_Y = 45.0f;
static constexpr float EDITOR_W = 609.0f, EDITOR_H = 331.0f;

static constexpr float KEYS_W = 36.0f, VSB_W = 13.0f, RULER_H = 16.0f;
static constexpr float ROW_H  = 9.0f;    // grid 315 px / 9 = 35 rows exactly
static constexpr float DEFAULT_PPS = 14.0f;   // 560 px / 14 = 40 steps = 2.5 bars
```

- Grid: y 61 → 376, height **315 = 35 × 9** ✓, width 609 − 36 − 13 = **560 px**.
- Editor bottom at 376, 3.4 px above the panel edge; x-disjoint from every screw.

The control bar is **32 px**, not the 24 px first proposed, so that Snap, eight
track squares and REC fit on one row (open decision 3 resolved: the controls stay
where they are and the roll pays for it). That costs the editor one pitch row —
**35 rows, 2.9 octaves**, against 36 at a 24 px bar. Row height stays 9 px per §7.1.

Jack centres:

```
clk_input       ( 26,  78)      voct_1_output (694,  68)   gate_1_output (734,  68)
rst_input       ( 26, 132)      voct_2_output (694, 108)   gate_2_output (734, 108)
rec_voct_input  ( 26, 202)      voct_3_output (694, 148)   gate_3_output (734, 148)
rec_gate_input  ( 26, 256)      voct_4_output (694, 188)   gate_4_output (734, 188)
                                voct_5_output (694, 228)   gate_5_output (734, 228)
                                voct_6_output (694, 268)   gate_6_output (734, 268)
                                voct_7_output (694, 308)   gate_7_output (734, 308)
                                voct_8_output (694, 348)   gate_8_output (734, 348)
```

The output columns were pulled in from the first proposal (698/738) to 694/734:
at 738 the gate column cleared the right-hand screws by **0.24 px**. The left column
at x = 26 straddles the screw column but is **y-disjoint** from both screws — safe,
but worth knowing it is safe by *coincidence of row placement*, not by margin.

Labels must be **paths, not `<text>`** — NanoSVG does not render text.

### 7.4 Panel construction follows the house style

Read off hazumi, note_detector and cue_research. Voxglitch panels ship **light and
dark themes** (`PanelHelper::loadPanel(light, dark)`, `ThemedSvgPanel`), with the
dark variant generated from the light one by `scripts/generate_dark_panel.py`:

| Element | Convention |
|---|---|
| Background | Flat **`#e7e7e7`**, authored in Inkscape as a solid swatch named *"LIght Panel Background"* |
| Control-group backing | **`#d2d2d3`** rounded rects, **rx 4.5**, behind clusters of jacks |
| Artwork, labels, logo | **`#0c1218`** paths |
| Jack markers | **`#213d42`** circles, ids ending `_input` / `_output` |
| Branding | VOXGLITCH wordmark plus a boxed module name, **top left** |

The placeholder now follows this: backing panels behind the clock pair, the record
pair and the eight track outputs; rx 4.5 throughout; and a reserved branding region
at x 4–48, y 8–50. **Labels and branding are left for the Inkscape pass** — they
have to be paths, and hand-authoring path outlines for text is not worth it.

The generator strips every `_input`/`_output` marker from the dark panel and maps
the colours above, so the dark variant needs no separate authoring.

---

## 8. The eight-track palette

Designed as a **4-column × lightness-ladder lattice on the dichromat blue–yellow
axis**, not eight points on a hue wheel: at 30% alpha over light lanes that axis is
the only chromatic signal a deuteranope or protanope has, and it supports about four
resolvable positions. Eight tracks share four CVD columns, separated within each
column by an 18–33 point L\* gap.

Measured (verified) worst-case separations, comparing all 16 dimmed swatches
(8 tracks × 2 lane types) rather than same-lane only:

| | This palette | vxsynth's 4 colours |
|---|---|---|
| Normal vision, dimmed | ΔE 5.93 | 8.93 |
| Protanopia, dimmed | **ΔE 3.14** | **0.41** |
| Deuteranopia, dimmed | ΔE 1.72 | 1.69 |

⚠ **CORRECTED:** the headline "fixes a latent defect" claim holds **only for
protanopia** (a genuine ~7.7× improvement — vxsynth's Purple/Blue pair is
effectively invisible to protanopes). For **deuteranopia the two palettes are
statistically identical**, and normal-vision separation actually *drops* by 34%,
because showing twice as many tracks costs separation. That is the honest trade.

### 8.1 Colour alone is not sufficient — two required mitigations

1. **Carry numerals on the track selector squares and consider per-track visibility
   toggles.** Seven simultaneous dimmed tracks is beyond what 30% alpha can carry
   for a dichromat, however the hues are chosen.
2. **Raise the dimmed fill alpha from 0.30 to ~0.40.** Separation is near-linear in
   alpha; at 0.40 every CVD pair clears ΔE ≈ 5 and normal vision clears 11. The
   active track still dominates via full opacity plus its stroke and grip hint.

Two further measured effects worth knowing: **the natural/sharp lane alternation
shifts every track's dimmed appearance by ΔE 2.2–2.6** depending on the pitch it
lands on, and **overlapping dimmed notes composite into false track identities** —
five combinations read as a *different* solo track at ΔE < 5 (e.g. Rose over Azure
reads as solo Purple). Per-track visibility toggles fix both.

Also: Rack applies a **global brightness tint** to everything a module draws
(`settings.hpp:58 rackBrightness`, plus Rack's NanoVG `nvgGlobalTint`), which
compresses exactly the lightness axis this palette depends on. Every figure above is
an upper bound.

---

## 9. Note identity and serialization

**Style:** raw jansson, `json_object_set_new` / `json_array_append_new`
**exclusively** — the house style (104 `_new` calls vs 25 bare, all of the latter
correctly paired with `json_decref`). This is what the recent
*"Fix Jansson refcount memory leak"* commit was about: the rule is **bare set/append
requires a matching decref**, and the leaks were on *named* variables, not only
inlined constructions.

**Schema:**

```json
{ "version": "1.0.0", "loop_length": 64, "active_track": 0,
  "notes": [ { "id": 6421953387712043, "pitch": 60, "start": 0, "length": 4, "track": 0 } ] }
```

**Identity:** an opaque **53-bit random integer** per note, minted once and frozen —
matching Rack's own `Module::id` convention. Chosen over a persisted counter because
`copyClipboard()` / `pasteClipboardAction()` round-trip through `toJson`/`fromJson`,
so a counter is copied verbatim into the pasted instance and collides. Chosen over a
36-char UUID string because it is one `uint64_t` compare in a 60 Hz redraw path
rather than a string hash. The selection becomes a `std::set<uint64_t>` of ids, **not
indices** — which retires the whole class of index-fragility documented in the
vxsynth editing document.

⚠ **CORRECTED — four traps, each of which would have shipped as a bug:**

1. **`rack::random::Xoroshiro128Plus` has no constructor.** The doc-comment example
   `Xoroshiro128Plus rng(rd(), rd())` at `random.hpp:16-18` is **stale** and does not
   compile under C++11 (which `compile.mk:21` mandates). The only seeding path is the
   `seed(uint64_t, uint64_t)` method.
2. **`std::random_device` is not guaranteed non-deterministic on MinGW-w64** — on
   older libstdc++ it is a fixed-seed mt19937 wrapper, so every instance would mint
   the *identical* id sequence. Mix in `Module::id` and `system::getUnixTime()`.
3. **`JSON::getInteger` returns `int`** (`JSON.hpp:23`), silently truncating a 53-bit
   id. Read ids with raw `json_integer_value`.
4. **The `JSON::` helpers return 0 on a missing key**, they do not leave the member
   unchanged — so `loop_length = JSON::getInteger(...)` yields 0, not the default 64,
   for any patch lacking the key. Set defaults in the constructor and check key
   presence explicitly.

**Load-side discipline:** clear notes and selection first (`dataFromJson` runs on
paste and preset-load into a populated module); bound the loop against `MAX_NOTES`;
range-validate and skip garbage rather than clamping silently; and if an id is 0,
missing, or duplicated, **re-mint and log a warning** — never throw, since an
exception out of `dataFromJson` takes the host down.

**Array order is not significant** under polyphony (last-note-priority is gone) and
the schema should say so.

---

## 10. Undo — resolved: use Rack's history system

⚠ **CORRECTED.** The earlier claim that *"Rack has no undo for arbitrary module
data"* is **false**. `history.hpp:63-65` says of `history::ModuleAction`: *"An action
operating on a module. **Subclass this to create your own custom actions for your
module.**"* `State::push()` is deliberately not marked `PRIVATE`, the symbols are
exported, and a custom-action translation unit was compiled against the real SDK to
confirm it links.

```cpp
struct NoteEditAction : history::ModuleAction {
    std::vector<Note> oldNotes, newNotes;
    std::vector<uint64_t> oldSelection, newSelection;
    NoteEditAction() { name = "edit notes"; }   // lowercase: reads as "Undo edit notes"
    void apply(const std::vector<Note>&, const std::vector<uint64_t>&);
    void undo() override { apply(oldNotes, oldSelection); }
    void redo() override { apply(newNotes, newSelection); }
};
```

Why Rack's system over a private stack: Ctrl+Z works with no key handling; the Edit
menu reads "Undo edit notes"; edits mark the patch dirty so Rack prompts to save; one
linear timeline instead of two competing ones; and delete-module-then-undo works for
free because actions reference the module by id.

**Push on gesture end, only if something changed** — snapshot in `onDragStart`,
push in `onDragEnd`. Never per `onDragMove`. **Store the selection in the action**:
undoing a Delete should give the selection back, which is materially better than the
original, and is only possible because notes now have stable ids.

**One `setNotes()` chokepoint** for both editor edits and undo — solve the UI/audio
race once, in one function.

⚠ **The proposed race fix was wrong and must not be used.** "Atomically swap a
`std::shared_ptr<const NoteList>` that `process()` reads" fails twice: pre-C++20
libstdc++ implements atomic shared_ptr with a **global spinlock table** (not
lock-free), and whichever side drops the last reference runs the deleter — which
would be `process()` calling `free()` on the audio thread. Use the repo's existing
**double-buffer + atomic index** pattern (`Kaiseki/AsyncSampleLoader.hpp:21-23`) and
free the retired buffer on the UI thread.

Two honest caveats. **Rack's history is global and linear**, so Ctrl+Z while hovering
the roll undoes whatever happened last *anywhere* in the patch — in vxsynth the roll
was effectively the whole app, so history was implicitly editor-scoped. And a private
stack would *not* "lose the user's work": Rack autosaves via `dataToJson()`
independently of history; the real benefit is the dirty flag and save prompt.

Do **not** use `history::ModuleChange` for note edits — two full JSON snapshots per
action, and undoing a note edit would also revert unrelated params.

---

## 11. The editor widget in Rack

Everything the web editor does is portable. Build it as **one
`rack::widget::OpaqueWidget`** covering keys + ruler + grid + scrollbar + corner,
with an internal `zone(Vec local)` dispatcher mirroring `_zone(lx, ly)` — do not
split zones into sibling widgets, or the precedence chain fragments.

| Need | Rack mechanism |
|---|---|
| Press + modifiers | `onButton`, `e.mods & RACK_MOD_MASK`, then `e.consume(this)` |
| Drag | `onDragStart` / `onDragMove` / `onDragEnd` — **never** teardown in `onButton(RELEASE)` |
| Mid-drag modifiers | `APP->window->getMods()` — `DragMoveEvent` carries no mods |
| Absolute position | `APP->scene->rack->getMousePos()` minus `getRelativeOffset(…)` |
| Cursor | `glfwSetCursor` directly — there is no Rack cursor API |
| Keyboard | `onHoverKey` — fires without focus, position-filtered, and `e.pos` is available |
| Wheel | `onHoverScroll`, always consume or `RackScrollWidget` zooms the patch |
| Menus | `createMenu()` + `createMenuItem` / `createSubmenuItem` / `MenuSeparator` |

Verified precedent: **eight leaf child widgets in this repo already override
`onHoverKey`** without focus or selection, so the keyboard route is established
practice here, not a gamble.

⚠ **Five corrections that matter:**

1. **The wheel sign is inverted between the two systems.** vxsynth zooms in on
   *negative* `deltaY`; Rack's convention (and `TrackWidget.hpp:404`) zooms in on
   *positive*. Porting the branch conditions literally **reverses every wheel gesture
   in the editor.**
2. **`OpaqueWidget::onDragHover` also auto-consumes.** An override for cursor updates
   must call the base or it silently loses DragEnter/DragLeave.
3. **`Widget::box` defaults to `Vec(INFINITY, INFINITY)`**, and `Rect::contains` has
   an explicit INFINITY branch — a widget that forgets to set `box.size` passes the
   hit test for *every* position in its parent and swallows all positional events.
4. **Use `RACK_MOD_CTRL`, never raw `GLFW_MOD_CONTROL`** — it maps to Super on Mac.
   And consume **selectively, key by key**: blanket-consuming would eat Ctrl+Z.
5. **`getMousePos()` has zero precedent in this repo** — all 19 existing drag sites
   use `e.mouseDelta` accumulation with `getAbsoluteZoom()`. The absolute-position
   idiom is inferred from headers, not proven here; verify it early, and keep
   accumulation as the fallback. This is the **single highest-risk unknown in the
   port**, because absolute tracking is required by the whole drag model.

Two smaller notes: an open context menu blocks the editor entirely (`onLeave` never
fires, so the cursor shape sticks), and **there is no double-click behavior to port**
— the vxsynth roll implements none.

---

## 12. Decisions resolved

All seven are answered.

**1. Light or dark editor ground — the editor is a DARK SCREEN, on both themes.**

The panel themes light/dark as usual (`ThemedSvgPanel`, generated by
`scripts/generate_dark_panel.py`), but the editor surface does not follow it. The
repo settled this: **no Voxglitch module reads `settings::preferDarkPanels`** — a
grep across `src/` returns nothing — and every custom-drawn display is dark
regardless of panel theme. CueResearch's `TrackWidget` fills itself
`nvgRGB(0x10, 0x20, 0x20)` while sitting on a light panel; GrooveBox's five
selectable LCD schemes are all near-black grounds. Voxglitch treats a display as a
*screen*, not as part of the panel.

The Piano Roll editor follows that, which has one significant consequence:

⚠ **The eight-track palette in §8 is invalid as designed.** It was tuned against
vxsynth's light paper lanes (`#e4e8ee` / `#d5dbe4`); the lanes are now
`#182a2a` / `#122222`. The hue lattice and the lightness-ladder method survive, but
**every ΔE figure must be recomputed against the dark ground**, and the direction of
the ladder likely inverts — on a dark lane, separation comes from getting *lighter*,
not darker. This has to be redone before notes are rendered.

The screen palette lives in one block at the top of `PianoRollEditorWidget`, so
retuning it is a single edit.

**2. `MAX_NOTES` raised to 2048.** 256 notes per track across 8, ~64 KB per undo
snapshot at ~32 bytes a note. Already applied.

**3. Controls stay in the control bar; the roll shortens to pay for it.** The bar
grows 24 → 32 px, the editor drops to 331 px, and the grid loses one row: **35 rows,
2.9 octaves** (§7.3). Already applied.

**4. Wrapped note tails are drawn at step 0.** vxsynth does not draw them, so its
audible steps are invisible — an audible/visible mismatch that polyphony makes
worse. The port draws the wrapped tail, which also makes the circular channel sweep
(§4.1) legible on screen rather than mysterious.

**5. Out-of-loop notes are marked — my call, resolved: yes, mark them.** They never
play, yet Shift can resurrect them at a scrambled position, irreversibly (§10.5 of
the editing document). Draw them **outline-only, no fill**, in their track hue: they
read as present-but-silent without competing with playing notes, and the treatment
costs one branch in the note draw. The alternative — leaving them looking normal in
a dimmed region — is what makes the vxsynth behavior surprising.

**6. Keys-column preview kept, sharing channel 0.** No channel reservation, so the
derived channel count never changes on a mouse-down and the shrink-quiesce in §4.4
stays purely edit-driven. The cost is that an audition collides with whatever voice
is sounding on channel 0 of the active track; that is the right trade against making
the count twitch on every key press.

**7. Exact-duplicate notes — allowed, with no special handling.** Two notes sharing
track, pitch, start and length are two real voices in unison under polyphony, and
they consume two channels. The channel sweep already counts them correctly, so
nothing breaks. No prevention, no warning, no detection — judged not worth the rule
the user would have to discover.
