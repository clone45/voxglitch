# tests/timeline

Out-of-Rack tests for the Timeline module (the vxsynth automation port).
The plugin itself is never built in this environment (see CLAUDE.md); these
compile the Rack-free parts directly.

    g++ -std=c++11 -O2 -I ../../src/modules/Timeline -o engine_test engine_test.cpp -lm && ./engine_test
    g++ -std=c++11 -O2 -I ../../src/modules/Timeline -o transport_test transport_test.cpp -lm && ./transport_test
    g++ -std=c++11 -O2 -I ../../src/modules/Timeline -o record_test record_test.cpp -lm && ./record_test
    g++ -std=c++11 -O1 -g -fsanitize=address -fno-omit-frame-pointer -I ../../src/modules/Timeline -o store_test store_test.cpp -lm && ./store_test

(chase_test and scrollbar_test build the same way as engine_test.
store_test is built WITH AddressSanitizer on purpose: its no-leak and
no-use-after-free claims are enforced by the tool.)

- **engine_test.cpp** — `TimelineEngine`: lane interpolation, the hold rules,
  cursor tracking through forward play and seek-back, the seek serial, loop
  wraps, tempo changes keeping musical position, the LaneData editing
  primitives, and the guards.
- **store_test.cpp** — `LaneStore`: publish swaps the pointer and retires the
  old snapshot; retire-by-generation frees only once the audio generation is
  more than RETIRE_LAG past the swap; the backlog stays bounded under an
  interleaved audio/UI simulation; the soft node cap (8192).
- **record_test.cpp** — latch-mode recording, both halves: the audio-side
  `Recorder` (grid capture at 1/4 and 1/32 on absolute grid lines, take
  start/end on arm+play, poly lane mapping, lane full, ring overflow) and the
  UI-side `TakeAssembler` (passed-over nodes replaced, untouched regions kept,
  loop-wrap sweep, the RDP simplifier keeping endpoints and bends). Its `Rig`
  mirrors the module's process() wiring and the editor's drain — re-sync if
  either changes.
- **chase_test.cpp** — centred-playhead scrolling: the two phases, zoom,
  loop wraps, and the suspend/resume rules. Also a COPY of the editor's
  `updateChase`. It already earned its keep: the resume rule was level
  triggered at first, so breaking the chase re-armed it one frame later.
- **scrollbar_test.cpp** — the editor's scrollbar geometry (thumb sizing,
  scroll<->thumb round-trip, clamping, paging, zoom). Also a COPY: the editor
  itself needs Rack, so re-sync if the geometry helpers change.
- **transport_test.cpp** — mirrors `Timeline::process()`'s decision structure
  (transport, internal/external tempo, song-clock pulses) with params and
  jacks stubbed. **If process() changes, re-sync this file** — it is a copy,
  not a call.

## Traps
- A param cannot be snapped from `process()`. Rack's `Knob` defaults to
  `smooth = true`, so it writes a target to the engine's parameter smoothing,
  and the engine drives the value every sample. Any rounding written from
  `process()` is overwritten before the next frame. Snap in a `ParamQuantity`
  subclass AND set `smooth = false` on the knob. `snapEnabled` only does
  whole integers.
- These tests build with g++ on Linux. The plugin builds with MinGW on
  Windows. MinGW still defines the DOS memory-model keywords `far` and
  `near`, so neither can be a variable name. A Linux-only test will not
  catch that.
- Rack's `math::clamp` is FLOAT-only. Beats are doubles (86,400 of them with
  sub-beat precision), so clamping a beat through it quantises node times.
  The editor uses its own `tlClamp`.
- The lane store is read by the audio thread while the UI edits it. All edits
  go through `laneCopy(L)` / `publishLane(L, copy)` (copy ONE lane, mutate,
  atomic pointer swap). Mutating `lane(L)` directly would race, and the audio
  thread must never free: retired snapshots are freed by `housekeep()` from
  the widget's `step()`, only once the audio generation has moved past them.
- Node storage is `float`. Compare recorded beats with a tolerance, and
  remember a capture on an existing beat lands to the RIGHT of that node.
- A recording take's nodes are simplified when it ends: a constant input
  collapses to two nodes. Count captures from the ring, not nodes in the
  lane, when testing the grid.
- A loop wrap must NOT bump the engine's seek serial — that difference is the
  only thing separating the RWND and LOOP jacks.
