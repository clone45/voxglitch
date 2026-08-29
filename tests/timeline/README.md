# tests/timeline

Out-of-Rack tests for the Timeline module (the vxsynth automation port).
The plugin itself is never built in this environment (see CLAUDE.md); these
compile the Rack-free parts directly.

    g++ -std=c++11 -O2 -I ../../src/Timeline -o engine_test engine_test.cpp -lm && ./engine_test
    g++ -std=c++11 -O2 -I ../../src/Timeline -o transport_test transport_test.cpp -lm && ./transport_test

- **engine_test.cpp** — `TimelineEngine`: lane interpolation, the hold rules,
  cursor tracking through forward play and seek-back, the seek serial, loop
  wraps, tempo changes keeping musical position, the LaneSet editing
  primitives, and the guards.
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
  go through `beginEdit()` / `commitEdit()` (copy, mutate, atomic publish).
  Mutating `lanes()` directly would race.
- A loop wrap must NOT bump the engine's seek serial — that difference is the
  only thing separating the RWND and LOOP jacks.
