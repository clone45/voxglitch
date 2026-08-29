## Timeline

Timeline is a 16-lane automation sequencer. Draw a control curve on a musical timeline — nodes joined by straight lines or bent into smooth curves — and play it back against the module's own tempo. All sixteen lanes leave through one polyphonic cable, one channel per lane, and a family of clock outputs lets you run sequencers and envelopes sample-locked to the curves by construction.

Each lane has its own transport: the polyphonic START, STOP and RESET inputs can run, halt and rewind every lane independently, so one Timeline can be sixteen loosely-coupled modulation sources rather than one rigid one.

### Quick Start

1. Draw a curve
   - Click in the editor to place a node. Keep holding and drag to position it.
   - Click again elsewhere to add more. Drag any node to move it; right-click a node to delete it.

2. Play it
   - Flip **PLAY**. The red playhead moves and the **LANES** output carries your curve on channel 1.
   - Patch **LANES** into anything with a CV input. Use a module like `SPLIT` to fan out the sixteen channels.

3. Bend a segment
   - Hover the line between two nodes: a diamond handle appears at its midpoint.
   - Drag the diamond and the segment curves to follow your pointer. There is a detent at straight; right-click the handle to snap back to a straight line.

4. Loop it
   - Flip **LOOP**. The loop end lands just past your drawn content, on a bar. Drag the amber handle in the ruler to move it.

### Tempo

Timeline owns its tempo: the **BPM** knob (20-300, in half-BPM steps) is the only clock source, and the clock outputs let everything else follow *it*. There is deliberately no clock input — patch Timeline's **CLK** out to whatever needs to stay in step.

### Inputs

All three transport inputs are polyphonic with the same rule: a mono cable is a broadcast to every lane, and a polyphonic cable addresses lanes individually — channel *n* drives lane *n*, and lanes past the cable's channel count are left alone.

- **START** - A trigger starts the lane(s) playing.
- **STOP** - A trigger stops them where they are.
- **RESET** - A trigger rewinds them to the beginning.

### Outputs

Every output is polyphonic, one channel per lane.

- **LANES** - The sixteen automation curves, -10V to +10V. An empty lane outputs 0V.
- **CLK** - A clock derived from each lane's playhead, at the division set by the **DIV** knob (1/1 bar down to 1/32).
- **RST** - Fires on a rewind *or* a loop wrap — the "start over" signal for downstream sequencers.
- **RWND** - Fires only on a user rewind (the RWND button, a RESET trigger, or scrubbing backward). A loop wrap does not fire it.
- **LOOP** - Fires only on a loop wrap.
- **RUN** - A gate, high while that lane is playing.

The RST/RWND/LOOP split lets a patch react differently to "the user started the song over" and "the loop came round again".

### Panel Controls

- **PLAY** - Starts and stops all lanes. Flipping the switch is a global gesture; between flips, the polyphonic inputs own each lane's state.
- **RWND** - Rewinds all lanes to the beginning.
- **LOOP** - Loops playback. The loop end is your drawn content rounded up to a whole bar (four bars if the timeline is empty), unless you have dragged the amber handle in the ruler.
- **CHASE** - The view follows the selected lane's playhead, keeping it centred. Deliberately moving the view — panning, paging the scrollbar — suspends the chase; it re-arms when you seek, when the loop wraps, or when the playhead leaves the screen and comes back. A red tick at the top centre shows the chase is active.
- **BPM** - Tempo, snapped to half-BPM steps.
- **SNAP** - The editing grid: off, 1 bar, 1/2, 1/4, 1/8 or 1/16. Governs node placement and the loop handle.
- **DIV** - The CLK output's division.

### The Lane Tabs

Sixteen numbered tabs above the editor choose which lane the editor shows. A tab with drawn content reads brighter, so you can see where you have already worked, and each tab carries a transport dot — green while that lane runs, red while it is stopped — making the tabs the one place all sixteen transports are visible at once.

### The Editor

- **Add** - Click empty space. The node lands on the snap grid and you can keep dragging it in the same gesture.
- **Move** - Drag a node. Dragging against either edge of the view auto-scrolls, so a node can be carried into empty future space without zooming out.
- **Delete** - Right-click a node.
- **Bend** - Hover a segment, drag the diamond. The curve is pulled through your pointer in both axes, like a rubber band. Right-click the diamond to straighten.
- **Scrub** - Drag the ruler at the top. Scrubbing seeks every lane.
- **Zoom** - Mouse wheel, anchored so the beat under the pointer stays put. When the rack itself is zoomed out, the wheel passes through to the rack.
- **Pan** - Middle-drag the editor body, or use the scrollbar at the bottom. The scrollbar always leaves a screenful of empty future beyond your content, so there is never a wall in the direction you compose.
- **Readout** - Bar.beat and minutes:seconds at the bottom right, following the selected lane.

A curve holds its first node's value before it and its last node's value after it, so a lane is defined everywhere. Each lane holds up to 256 nodes.

Every completed gesture is one undo step.

### Right-Click Menus

On the editor:

- **Clear lane** - Removes every node from the selected lane.
- **Straighten lane** - Removes every bend, keeping the nodes.
- **Fit view to content** - Zooms to show everything you have drawn.
- **Reset zoom** - Back to the default view.

On the panel: **Lock Editor** freezes the editor against mouse edits — the display takes a red tint, and clicks fall through to the panel. Unlock from the same menu.

### Tips

- Sixteen independent transports make Timeline a bank of one-shot envelopes: draw a shape per lane, leave PLAY off, and fire individual lanes with triggers into the polyphonic START input.
- RST fires on both rewinds and loop wraps, so it is usually the one to patch into a sequencer's reset.
- The CHASE tick disappearing tells you why the view stopped moving: you moved it, and the chase is waiting to re-arm.
- Lock the editor once a piece is finished — a stray click cannot then add a node in the middle of your automation.
