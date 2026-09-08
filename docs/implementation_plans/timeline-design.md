# Timeline — the vxsynth automation system as a Rack module

2026-08-27, branch `dev/timeline`. Port of the vxsynth studio's global
automation timeline (`~/vxsynth/app/studio/lib/audio/wasm/core_head.c`, the
AUTO A-D readers, Song Clock, and `lib/automation/timeline-editor.js`).
NOTE: `../vxsynth` on this machine is the STALE frozen rollback; the live
source of truth is `~/vxsynth`.

## What the web version is
One GLOBAL musical-time playhead plus 32 sparse breakpoint lanes (4 banks x
8, 256 nodes each). `automation_tick()` runs once per sample before module
dispatch, advances a beat accumulator, handles the loop wrap, and
re-evaluates every lane by linear interpolation with a tracked cursor (no
per-sample search, correct on backward seeks). AUTO A/B/C/D are thin readers
of an 8-lane slice; Song Clock derives CLK pulses from the same accumulator,
so anything it clocks is sample-locked to the curves by construction. A
`seekSerial` counter distinguishes a USER seek (serial bumped) from a LOOP
wrap (playhead jumps back inline, no bump) — that one trick is what lets
RWND and LOOP be separate jacks.

## What changes in the port (Bret's calls)
- **Self-contained, not global.** Rack has no per-patch global state, and
  several Timelines in one rack is a feature, not a bug. Each instance owns
  its playhead, its lanes and its editor. The four AUTO expander readers are
  gone: this module has the outputs on its own faceplate.
- **16 lanes, one 16-channel POLY output.** Rack's poly maximum is exactly
  16, so the whole timeline leaves on one cable. Lanes 1-8 originally also
  had mono jacks; user testing (2026-08-28) said the poly cable alone was
  sufficient, so the eight jacks were removed. Split with any poly utility.
- **Internal BPM only** (matching the web original, which has no clock
  input: the dock's BPM field is the sole tempo source and Song Clock EMITS
  clock). A CLK input was built and then REMOVED 2026-08-27 at Bret's call.
  BPM snaps to 0.5.
- **Song-clock outputs on board**: CLK, RST, RWND, LOOP, RUN.
- **Transport jacks + buttons**: START, STOP, RESET.
- **Loop** with a draggable end handle in the ruler.

## Time base (kept verbatim from the web engine)
Beats, as doubles, 4/4. Nodes, loop end and playhead all live in musical
time; tempo only scales the per-sample increment, so a BPM change stretches
the song and never moves a node. Wall-clock is a derived view
(`sec = beat * 60 / bpm`). Hard ceiling 86,400 beats (12 h at 120 BPM),
256 nodes per lane.

## Anti-pop / correctness contract
- The playhead is an ACCUMULATOR (`+= beatsPerSample`), never `T * scale`
  recomputed from a measured interval — that rescaling was the root cause of
  Scatter's clock-edge pops, and automation CV feeds other modules' inputs
  where a step is just as audible downstream.
- Lane edits happen on the UI thread while the audio thread reads. Edits go
  through a DOUBLE BUFFER with an atomic pointer swap (single writer), so the
  audio thread never sees a half-written lane.

## Files
- `TimelineEngine.hpp` — lanes, playhead, eval, loop, seek. No Rack
  dependencies, so it is testable out-of-Rack (tests/timeline/).
- `Timeline.hpp` — the Module: params, jacks, transport, clock, outputs.
- `TimelineEditor.hpp` — the NanoVG editor widget (zoom, pan, hit-test,
  snap, undo).
- `TimelineWidget.hpp` — panel assembly.

## Musical vs Timecode (investigated 2026-08-27)
Bret recalled a song/timecode option. It exists in vxsynth, but in the TRACKS
module, not the automation timeline:
- **Time Mode: Musical / Timecode** (tracks.md) — whether a clip keeps its
  bar-and-beat position or its wall-clock position when the tempo changes.
  The ruler follows the mode: bars/beats vs seconds.
- **Clock: Own / Song** (tracks.md) — whether Tracks runs its own transport
  or hands it to the automation timeline.
The automation timeline itself is musical-time ONLY: `automation.md` never
mentions timecode, and neither the dock nor timeline-editor.js carries a
`_timeMode`. Adding a Timecode mode to Timeline would therefore be a NEW
feature, not parity. It is a coherent one (nodes pinned to wall-clock for
hits locked to picture, with a seconds ruler), and the engine change is
small — store node times in seconds and convert, or keep beats and scale on
tempo change. Awaiting Bret's call.

## The scrollbar (2026-08-27)
The first build inherited Tracks' pan strip — a featureless bar you drag
anywhere, with a rewind button at its left end. Bret asked for a conventional
scrollbar instead, so the strip is now one: the thumb's LENGTH shows how much
of the content is on screen and its position shows where you are, dragging it
pans, and clicking the track pages by 90% of a screen. Middle-dragging the
body still pans, and rewind is already on the panel's RWND button, so nothing
was lost.

The scrollable extent is never the engine's 86,400-beat ceiling — a thumb
sized against the ceiling would be a pixel wide. The thumb keeps a 24 px
minimum so it stays grabbable on a long song, and zooming re-clamps the
scroll so the thumb can never leave the track. Geometry verified in
tests/timeline/scrollbar_test.cpp.

**Elastic extent (2026-08-27).** Clamping the extent to existing content —
correct scrollbar semantics — walled users out of the empty future where they
wanted to place the NEXT node; reaching it meant zooming out until it came
into view. The extent is now `max(content, view end) + one screen`, so there
is always exactly one screen of future and never a wall; it grows as you
travel, and the thumb shrinks and approaches, without reaching, the right
end. While the thumb is dragged the extent is LATCHED: an extent that grows
mid-drag changes scroll-per-pixel under the pointer and the thumb
rubber-bands away from the cursor. Companion: dragging a NODE against either
edge auto-scrolls the view, so a node can be carried into empty space without
interrupting the gesture.

## Editor lock (2026-08-27)
Right-click the module: **Lock Editor**, the same pattern Tracks uses
(a `locked` bool on the module, a `createBoolMenuItem`, guards in the event
handlers, and a faint red tint over the editor). Persisted with the patch.

What it blocks: everything that CHANGES something — adding, moving and
deleting nodes, Clear lane, dragging the loop handle, and scrubbing the
ruler. Node hover highlighting is suppressed too, so nothing looks grabbable
that is not.

The wheel passes through to Rack when the rack is zoomed out below 0.95
(the Tracks threshold). At that size the editor is too small to aim at, and
swallowing the wheel would trap the user's view scroll. The pass-through
works by returning WITHOUT consuming the event.

What it allows: all navigation — zoom, scrollbar, paging, middle-drag pan,
lane tabs, and the panel's own transport controls. A locked timeline is still
readable, which is the point: the lock protects the drawing, not the view.

## Per-lane transport (2026-08-28)
Users asked for polyphonic START/STOP/RESET — each lane independently
startable, stoppable and rewindable. That made a real `Lane` class the
correct shape, not a style choice: every lane now owns its own playhead,
playing flag, cursor, seek serial and current value.

**The ownership split that survives the refactor:** a Lane owns its
PLAYBACK; the NODE DATA stays in the double-buffered LaneSet. If playback
state lived in that buffer, every edit would copy stale playheads over live
ones. A Lane knows its own index and evaluates against whichever LaneSet the
caller passes, so the buffering stays invisible to it.

The rules (Bret's calls):
- **Poly inputs:** 1 channel = broadcast to every lane; N channels = channel
  c drives lane c; lanes at or beyond N are untouched.
- **All six output jacks are polyphonic**, one channel per lane — the lane
  values and CLK/RST/RWND/LOOP/RUN alike.
- **Panel gestures are global:** the PLAY switch, RWND and the ruler scrub
  act on every lane. The poly inputs are the only per-lane address.
- **The editor follows the SELECTED lane:** playhead line, readout and chase
  all show the lane the tabs have picked; switching tabs re-primes chase so
  the playhead swap is not mistaken for a rewind. Each tab carries a small
  run dot, the only place all 16 states are visible at once.
- **Global stays global:** BPM, SNAP, DIV, and ONE loop length — each lane
  wraps at its own arrival at the shared loop end.
- **The PLAY switch is an EDGE, not a level.** Flipping it starts or stops
  every lane; between flips the poly inputs own each lane's state. A level
  would overwrite per-lane control every sample.

Testing traps met during the re-sync (details in tests/timeline/README.md):
a Schmitt trigger starts HIGH, so the first-ever sample on a fresh channel
cannot fire — simulations must prime each trigger with one low sample, as a
real cable does; and the parked-playhead rule fires the beat-0 downbeat when
play starts, so 4 beats of travel produce 5 CLK pulses.

## Open
Panel size (starting 48 HP), curve shapes beyond linear (the web version is
linear only), whether lanes get names, per-lane output attenuation.

## Port into the voxglitch collection (2026-08-28)
Timeline was built in `voxglitch_devices` (branch `dev/timeline`,
d852495..9f5fc8c) and moved here to be released free under the collection's
GPL-3.0-or-later licence. The engine, module and editor ported UNCHANGED —
they carried no repo-specific dependency. Only the widget and the packaging
changed:

- **Positions come from the SVG.** Every control is placed by
  `panelHelper.findNamed("<id>")` against a named circle in
  `res/modules/timeline/timeline_panel.svg`. Move a circle in Inkscape and
  the control follows. The 16 ids are the contract between art and code:
  start_input, stop_input, reset_input, rewind_button, play_switch,
  loop_switch, chase_switch, bpm_knob, snap_knob, div_knob, clk_output,
  rst_output, rwnd_output, loop_output, run_output, lanes_output.
  The light and dark panels must keep identical anchor positions or the dark
  theme shifts the controls.
- **Coordinates transferred exactly.** The panel is `width="243.84mm"` with
  `viewBox="0 0 720 …"`, which parses to 720 px at Rack's 75 dpi, so one SVG
  user unit is one Rack pixel and the old hardcoded numbers became anchor
  positions untouched.
- **House components:** `VoxglitchPolyPort` for the polyphonic inputs and the
  LANES output, `VoxglitchOutputPort` for the clock family, `squareToggle`
  for PLAY/LOOP/CHASE, `VCVButton` for RWND (this collection uses VCVButton
  and LEDButton nowhere).
- **The DRM overlay is gone.** This collection has no licence gate.

### Labels belong to the art
The control labels are NOT drawn in code. nanosvg cannot render `<text>`, so
the collection carries labels as outlined paths from a vector editor, and
Timeline follows that: the panel art owns every control label. The code-drawn
labels that came over from the devices build were removed 2026-08-28.

The only typography left in code is the wordmark at the top left, "TIMELINE /
AUTOMATION", which Bret kept.

So the panel art still to be drawn is: the sixteen control labels, and
whatever styling the background and the output plate deserve. The anchors
already carry every position, so none of that touches code.

## Segment bends (2026-08-28)
Users asked for curves between points (one cited Entrian's ctrl-J/ctrl-S
keystroke cycling). Bret wanted a visual gesture, not hidden chords, with an
easy way back to a straight line.

**The gesture.** Hover a segment and a small DIAMOND appears at its midpoint,
sitting on the curve — a different shape from the round nodes, so it reads as
a different kind of thing. Drag it up or down and the curve bends to follow
the pointer: toward one node is exponential, past straight and onward is
logarithmic. One axis, one parameter, continuous — the DAW convention
(Ableton/Bitwig/FL bend segments the same way).

**Back to straight is part of the gesture:** the drag has a DETENT — within
|bend| < 0.12 of the linear midpoint it snaps to exactly 0. Right-clicking
the handle straightens that segment; the canvas menu gains "Straighten lane".

**The maths.** One bend per segment, stored on the LEFT node:
value = v0 + (v1-v0) * frac^(2^bend), bend clamped [-3, 3] (exponent 1/8..8).
The editor inverts the midpoint the user drags: u = (m-v0)/(v1-v0),
k = ln(u)/ln(0.5), bend = log2(k). A straight segment skips the pow entirely,
so lanes without curves cost exactly what they always did. Endpoints are
exact for any bend, so bending NEVER creates a discontinuity at a node.

**Editing invariants:** the bend travels with its left node through insert
(splitting a bent segment: the left half keeps the bend, the new node's
segment starts straight), erase, and resort. Undo snapshots carry bends.
Persistence writes "b" per node only when nonzero; old patches load with
every segment straight.

**Click priority:** the handle wins over add-node, but ONLY the hovered
segment shows a handle, so clicking the curve anywhere else still adds a
node, exactly as before.

Deliberately out of v1: S-curves. They need a second parameter and a second
gesture. If they earn their way in, shift-drag on the same diamond is the
natural slot.

### Bend revision after the feel test (same day)
Bret: "It doesn't quite feel right… I can only move the diamond up and down.
And when I move it up all the way, the shape feels weird." Two separate
flaws, one revision:

- **Family swap.** frac^k grows a CORNER at high exponents — near-vertical
  at one end, flat everywhere else (his screenshot). Replaced with the
  exponential-approach (RC-charge) family
  `f_b(x) = (1-e^(-bx))/(1-e^(-b))`, b in [-10,10]: saturates smoothly at
  extremes, and the exp and log sides are exact mirrors
  (`f_-b(x) = 1 - f_b(1-x)`), which the power family never was.
- **2D solve.** The drag no longer reads only the pointer's height at the
  midpoint: it solves b so the curve passes THROUGH the pointer, both axes
  (f_b is monotone in b for fixed x, so 40 bisection steps, no derivative,
  cannot diverge). The handle rides the curve at the pointer during the
  drag. It feels like pulling a rubber band.
- Detent widened to |b| < 0.4 to suit the new parameter's scale.

The bend's stored meaning changed with the family. No compatibility shim:
the old meaning existed for one unreleased commit.

## Recording and the per-lane store (2026-09-03)

Two changes that had to land together: automation RECORDING (latch mode),
and the lane store it needed. Owner decisions are Bret's, 2026-09-03.
Timeline shipped in v2.44, so every param, input and output id that
existed keeps its numeric value; `REC_PARAM` and `REC_INPUT` are APPENDED,
and every patch saved by v2.44-2.46 loads unchanged.

### Why the store changed
The old store was `LaneSet laneBuf[2]`: 16 lanes x 256 nodes x 3 doubles,
double-buffered, published wholesale by one pointer swap. The 256 budget
was never a design decision here — it was inherited from the web engine's
fixed C arrays. Recording at 1/32 makes 256 nodes a 32-bar ceiling, so the
budget had to go, and with it the fixed arrays: a vector-backed LaneSet
copied on every drag would allocate 16 lanes to move one node.

### The store now
- `LaneData` — ONE lane: three `std::vector<float>` (t, v, b), immutable
  once published. The same helpers the editor used (insert, erase, resort,
  indexOf, eraseRange, lastBeat, count) operate on one lane.
- `LaneStore` — `std::atomic<const LaneData*> live[16]`, plus an
  `audioGeneration` counter the audio thread increments once per
  `process()` (and once per `processBypass()`, so a bypassed module keeps
  the clock moving).
- **Audio thread:** loads the 16 pointers once per sample, reads through
  them, never allocates, frees, or touches a refcount.
- **UI thread:** `laneCopy(L)` -> mutate -> `publishLane(L, copy)`. The
  copy is heap-allocated, swapped in with one atomic exchange, and the OLD
  pointer goes on a retire list stamped with the generation at retirement.
  `housekeep()` (from the widget's `step()`) frees a retired snapshot once
  `audioGeneration > retire_gen + 2`: the audio thread has provably run two
  full blocks past the swap. `onRemove` frees all retired; `~LaneStore`
  frees everything. Module removal does not leak (store_test builds under
  AddressSanitizer to prove it).
- **Why retire-by-generation, not shared_ptr:** the audio thread must
  never free (a free is a lock and an unbounded stall), and refcount
  traffic on every sample is the wrong price for a pointer that changes a
  few times a second. One relaxed increment per block is the whole
  audio-side cost.
- Per-lane edits copy only that lane. `LaneEditAction` holds a vector of
  (lane, before, after), so a mouse gesture is one entry and a recording
  take is one action across every lane it recorded.
- `TL_MAX_NODES` is now a SOFT cap of 8192 per lane: the JSON load bound
  and the "lane full" state. Where the 256 / fixed-array assumptions lived
  and what replaced them: `LaneSet`'s `count[16]` + `t/v/b[16][256]` ->
  `LaneData` vectors; `LaneSet::add/insert` returning false/-1 at 256 ->
  `LaneData::full()` at the soft cap; `laneBuf[2]` + `editBuf` +
  `beginEdit()/commitEdit()/setLanes()/lanes()` -> `LaneStore` +
  `laneCopy()/publishLane()/lane()/lastBeat()`; `Lane::eval(const LaneSet&)`
  indexing `ls.t[idx][c]` -> `eval(const LaneData*)`; `TimelineEngine::tick`
  taking one LaneSet -> an array of 16 const pointers; the editor's drag
  re-find loop -> `LaneData::indexOf`; `LaneEditAction`'s six double vectors
  for one lane -> a vector of (lane, before, after) LaneData; `dataFromJson`
  building a LaneSet -> per-lane LaneData bounded by the cap; the manual's
  "up to 256 nodes"; and the tests' `LaneSet` fixtures -> a `Set` of 16
  LaneData with a pointer array.
- Storage is `float`. Beats to 86,400 keep ~0.008-beat precision at the
  ceiling, far finer than any grid; volts and bends never needed doubles.
- JSON: same top-level shape (`loopEnd`, `locked`, `lanes`: 16 arrays of
  `{t, v[, b]}`), arrays may exceed 256, plus `record_rate` (absent in
  old patches -> default 1/4).

### Recording: latch mode, the only mode
- `REC_PARAM` (a switch, "Off"/"Armed", red squareToggle variant local to
  the Timeline widget) and `REC_INPUT` (poly). A TAKE runs while REC is
  armed AND a target lane is playing; it starts on the first sample both
  are true and ends when either goes false. Arming while stopped waits
  for PLAY.
- Targets: REC IN mono or unpatched -> the selected lane; poly with N
  channels -> lanes 0..N-1, each from its channel — but only those that
  are PLAYING at the take start. A parked lane has no sweep to record,
  and bypassing it for the whole take would silence it for nothing
  (review finding, 2026-09-03). The target set is FROZEN at the take
  start (switching tabs mid-take does not move the take). An unpatched
  REC IN records 0 V — still a take.
- Record rate: a module setting (not a param), persisted as
  `"record_rate"`, index into {1 bar, 1/2, 1/4, 1/8, 1/16, 1/32}, default
  1/4. The grid is ABSOLUTE beat positions, not relative to the take.
- Audio side (`Recorder`, Rack-free): at the take start, at every grid
  line a target lane's playhead crosses, and at the take end, push
  `(lane, beat, volts)` into a single-producer single-consumer ring
  (`CaptureRing`, 8192 fixed entries; overflow drops the NEWEST and counts
  it — overwriting the oldest would write into the slot the consumer may
  be copying, and a torn capture is worse than a missing one). A loop
  wrap is flagged on the next capture with the loop end it crossed, so the
  drain can sweep `(prev, loopEnd]` then `[0, B]`. A seek (serial bump; a
  wrap does not bump it) is flagged too, and that capture lands on the
  seek target rather than the grid cell below it.
- Bypass: while a lane is being recorded its poly output IS the live REC
  IN voltage, so the user hears what they record with no UI latency. The
  lane's playhead keeps running underneath; when the take ends the lane
  returns to its nodes.
- UI side (`TakeAssembler`, drained from the editor's `step()` — the
  PianoRoll idiom): each capture removes the nodes with
  `prev_B < t <= B` (the ones the playhead passed over), writes a node
  ONLY IF THE VALUE CHANGED (Bret, 2026-09-03; tolerance 0.001 V against
  the last node the take wrote), and publishes through the pointer-swap
  path so playback and the drawing follow within a frame. The audio side
  keeps pushing every grid capture regardless, because the sweep must
  still erase the passed-over region under a static input. What gets
  written: the take START (anchors the take); at a change, the grid point
  BEFORE it with the old value (the "hold node", so a hold stays flat up
  to the change instead of ramping across the gap) and then the new
  node; the take END (pins where the latch stops, skipped only when the
  previous capture already put a node on that beat). A hold spanning a
  loop wrap is pinned at the last grid point before the loop end, and the
  wrap capture starts a new hold. Seek captures follow the same rule
  after their single-node erase. Only beats actually inserted go on the
  take's `captured` list, so the simplifier never sees a phantom. Nodes not yet reached are
  untouched: THAT is latch mode. A seek mid-take, in EITHER direction,
  replaces only the node on the new beat and restarts the sweep there:
  nothing between the old and new positions was passed over, so a
  forward scrub must not erase it. An END on the beat of the previous
  capture is not inserted (it would only duplicate that node).
- Take end: Ramer-Douglas-Peucker over the recorded region, vertical
  deviation, tol 0.02 V, considering ONLY the nodes this take inserted
  (each lane's take keeps the list of beats it captured): after a wrap
  or a seek the bounding interval contains stretches the playhead never
  passed, and the hand-placed nodes there are the latch promise. The
  take's first and last node and any node carrying a bend are never
  removed either. Then ONE undo action for the whole take (per lane:
  before = the lane at the take start, after = simplified). The editor
  pushes a finished take BEFORE applying the next take's START as well as
  after each drain, since both can arrive in one frame and applying the
  START first would discard the finished take's `before`.
- Lane full: a lane at the cap stops capturing for the rest of the take,
  and the readout shows a red LANE FULL until the next take or edit on it.
- The editor lock does not block recording: arming REC is a deliberate
  panel gesture, and the bypass means the user would otherwise hear a
  take that silently recorded nothing.

### Panel and menu
SNAP and DIV left the panel (their params keep their ids, persisted and
MIDI-mappable, and are set from the context menu: Snap, Clock division,
plus Record rate). `rec_input` sits right of `reset_input` at the 40 px
jack pitch; `rec_switch` between PLAY and LOOP; the transport group
RWND · PLAY · REC · LOOP · CHASE moved right at its 34 px pitch and BPM
with it. New anchor centres (y 356): rec_input 150, rewind_button 194,
play_switch 228, rec_switch 262, loop_switch 296, chase_switch 330,
bpm_knob 394; every other anchor unchanged. The REC labels are Pilat
outlines from `scripts/generate_vx_drums_panels.py`'s `text_paths`, same
size (7.29 px), tracking and baseline (334.60) as the hand-authored
neighbours; the dark file carries them in `#f4eee9`.

### Tests
`tests/timeline/store_test.cpp` (publish/retire/no leak under ASan, the
cap) and `record_test.cpp` (grid capture at 1/4 and 1/32, loop-wrap
replacement, latch leaves untouched regions, the simplifier, poly lane
mapping, bypass, lane full, ring overflow). The four existing tests moved
to the new store and still pass.
