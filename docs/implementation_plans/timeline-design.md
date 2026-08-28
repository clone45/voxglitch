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
