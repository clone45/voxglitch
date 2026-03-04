# GAPads

An interactive pad controller module that outputs the state of 16 on-screen pads as a polyphonic signal. Each pad can operate in toggle or momentary mode, and its ON/OFF state is represented as a gate voltage on a dedicated polyphonic channel. This module bridges the visual pads interface in GrooveboxAdvanced with the internal patching system, letting users trigger sounds, control parameters, or build interactive performance patches.

## Inputs

None.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | Out | Gate | Polyphonic 16-channel output. Each channel corresponds to one of the 16 pads (channel 0 = pad 1, channel 15 = pad 16). A pad in the ON state outputs 10V; a pad in the OFF state outputs 0V. |

## Parameters

None. Pad states are controlled interactively through the on-screen pads UI rather than through configurable knob parameters. Each pad's behavior mode (toggle or momentary) is set via right-click context menu on the pad itself.

## Details

### Signal Flow

The GAPads module acts as a bridge between the graphical pads interface (GAPadsCanvas) and the internal DSP signal graph. The data flow works as follows:

1. **User Interaction**: The user clicks pads on the GAPadsCanvas, which is a 2x8 grid of 16 interactive pads displayed in the GrooveboxAdvanced UI. Left-click toggles a pad (in toggle mode) or holds it active while pressed (in momentary mode). Right-click opens a context menu to switch between toggle and momentary modes or reset the pad.

2. **State Transfer**: The main GrooveboxAdvanced processor reads the pad states from the host module and writes them directly into the `padStates[16]` array on the GAPadsDSP object before each processing cycle. This bypasses the normal `syncToDSP` parameter path since pad states are managed at the host level rather than as serialized module parameters.

3. **DSP Output**: During `process()`, the module iterates over all 16 pad states and writes the corresponding voltage to each channel of a polyphonic output signal. The output always has exactly 16 channels. Each channel outputs either 10V (pad ON) or 0V (pad OFF), using VCV Rack's standard gate voltage level.

### Pad Modes

Each pad supports two interaction modes, configurable per-pad via right-click context menu:

- **Toggle Mode** (default): Clicking the pad flips its state between ON and OFF. The state persists until clicked again.
- **Momentary Mode**: The pad is ON only while the mouse button is held down. Releasing the button or dragging off the pad returns it to OFF. The UI displays a fade-out animation when the pad is released.

### Stateless DSP

The DSP module itself has no internal state beyond the `padStates` array, which is overwritten each processing cycle by the host processor. The `reset()` method clears all 16 pad states to false (0V). There is no `syncFromDSP` path because no DSP-computed values need to propagate back to the UI.

### Polyphonic Signal Structure

The output is a `PolySignal` with `channelCount` fixed at 16. Downstream modules that read specific polyphonic channels can extract individual pad states. When connected to a mono input, only channel 0 (pad 1) will be read. To access specific pad channels, use the output with modules that support polyphonic input or that can select individual channels.

## Tips

- Patch the poly-16 output into a mixer or VCA bank to use pads as a manual mute/unmute controller for up to 16 sound sources.
- Use individual pad channels as gate inputs to envelope modules: toggle a pad ON to trigger an envelope, creating a simple manual keyboard-style performance interface.
- In momentary mode, pads function like drum trigger buttons. Patch them into triggered sample players (GATrigSample) for finger-drumming style performance.
- Combine with a GALogic module to create composite conditions from multiple pad states, for example requiring two pads to both be ON before passing a signal.
- Use a pad in toggle mode as an enable/disable switch for effects: patch it into a VCA controlling a delay or reverb send to toggle effects in and out during a performance.
- Pair with GASwitch or GASwitch3 modules to use pad states as selection signals, routing audio or CV to different destinations based on which pads are active.
- Since all 16 pad states are available simultaneously on one polyphonic cable, a single GAPads module can control many aspects of a patch without cluttering the canvas with multiple control modules.
