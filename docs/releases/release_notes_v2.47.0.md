# Voxglitch 2.47.0

Now available in the VCV Rack library.

## VX Drums

- **User kit library.** Save your own kits from the right-click menu with **Save kit as**, then **Update** and **Manage kits** to keep them tidy. Saved kits are offered by every VX Drums instance, including ones you add later.
- A saved kit carries all six drum models plus the 24 voice knobs.
- **Patches are self-contained.** A patch stores a copy of its kit, so it loads and sounds right on a machine that does not have that kit installed.
- **Stereo panning** switch in the menu, for the Machine's fixed per-voice pan positions.
- The six model submenus are now grouped under **Kit Customization**.

## VX Drum Sequencer

- **Chaining by adjacency.** Place sequencers side by side and they chain automatically: the leftmost is the head, members play in turn, and every member mirrors TRIG and ACC so you can patch the kit from whichever module sits nearest it.
- **Per-pad chance.** Give any pad a probability, rolled once per step. Set it from the pad menu, or switch to the new **Chance** mode where lit pads become draggable bars.
- **Clear pattern** on the panel menu.
- **Randomize settings**, to choose which lanes the RND button is allowed to rewrite.

## Timeline

- **Curve recording.** Patch a CV source into the new **REC IN**, arm **REC**, and press play. Whatever arrives is written into the lane as the playhead passes.
- **Recording is a latch, not a wipe.** Only the part the playhead actually passes over is overwritten, so you can punch in over a section, or go round a loop several times building a lane up a piece at a time. Each take is one undo step.
- **Polyphonic recording.** A poly cable into REC IN records channel 1 into lane 1, channel 2 into lane 2, and so on, up to 16 lanes in a single take.
- **Record rate** menu setting, 1 bar down to 1/32. Nodes are written only where the value changes, so a steady voltage records as just two nodes.
- **Automatic tidy-up** at the end of a take removes redundant nodes, so a slow drift becomes a few segments instead of one node per grid line. Nodes you placed by hand are never touched.
- **Live monitoring.** While a lane records, its output is the incoming voltage, so you hear exactly what you are recording with no delay.
- **Per-lane transport.** Each lane tab carries a dot, green while that lane runs and red while it is stopped, so all sixteen transports are visible at once.
- **Lane capacity raised from 256 to 8192 nodes**, with a LANE FULL notice if a take reaches the ceiling.
- SNAP and DIV moved off the panel into the right-click menu as **Snap** and **Clock division**. Existing patches keep their settings.

## Compatibility

Older patches load unchanged. VX Drums patches upgrade to kit format 1.2.0 and VX Drum Sequencer patches to 1.1.0, and existing clips and pattern files still load.
