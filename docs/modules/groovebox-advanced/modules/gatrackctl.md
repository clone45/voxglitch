# GATrackCtl

A track control module that provides per-track volume, pan, and mute control via polyphonic CV inputs. It lives in the Sequencer Control global patch and allows internal patch logic or external signals to dynamically adjust the mixer parameters for each patch slot in the patch library. Each of its three polyphonic inputs independently controls the corresponding parameter across up to 16 tracks.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | VOL | Control (CV) | Polyphonic volume input. Each channel's voltage is scaled from 0-10V to a 0.0-1.0 volume range and clamped to that range. When monophonic, channel 0's value broadcasts to all 16 tracks. When disconnected, volume values are set to -1.0 (sentinel), meaning no override is applied and existing patch library volume settings are preserved. |
| 1 | PAN | Control (CV) | Polyphonic pan input. Each channel's voltage is scaled from 0-10V to a 0.0-1.0 pan range, where 0.0 is hard left, 0.5 (5V) is center, and 1.0 is hard right. Values are clamped to 0.0-1.0. When monophonic, channel 0's value broadcasts to all 16 tracks. When disconnected, pan values are set to -1.0 (sentinel), preserving existing patch library pan settings. |
| 2 | MUTE | Control (CV) | Polyphonic mute gate input. Each channel is compared against a 0.5V threshold: voltages above 0.5V mute the corresponding track, voltages at or below 0.5V leave it unmuted. When monophonic, channel 0's value broadcasts to all 16 tracks. When disconnected, all tracks default to unmuted (false). |

## Outputs

None.

## Parameters

None.

## Details

### Signal Flow

The GATrackCtl module acts as a bridge between CV signals in the Sequencer Control global patch and the host's per-track mixer parameters. The signal path is:

1. **CV Input Reception**: Three polyphonic input ports (VOL, PAN, MUTE) each receive up to 16 channels of CV. Each channel corresponds to a patch slot index in the patch library.

2. **Voltage Scaling and Clamping**: In the `process()` method, the module iterates over all 16 possible track channels for each input:
   - **Volume**: `clamp(voltage / 10.0, 0.0, 1.0)` -- maps the standard 0-10V CV range to a normalized 0.0-1.0 volume scalar.
   - **Pan**: `clamp(voltage / 10.0, 0.0, 1.0)` -- maps 0-10V to a pan position where 0.0 is hard left, 0.5 is center, and 1.0 is hard right.
   - **Mute**: `voltage > 0.5V` -- a simple gate threshold comparison producing a boolean mute state.

3. **Connection Tracking**: The module tracks whether each input port has a cable connected. When an input is disconnected, its corresponding values are set to sentinel values: -1.0 for volume and pan (meaning "no override"), and false for mute. This allows the module to selectively override only the parameters that have active cables.

4. **Mono Broadcasting**: The `PolySignal::get(c)` method handles mono-to-poly broadcasting automatically. When an input has only 1 channel, requesting any channel index returns channel 0's value. This means a single mono CV cable controls all 16 tracks uniformly for that parameter.

5. **Host Consumption**: After the Sequencer Control patch processes, the `GAProcessor::getTrackCtlOutputs()` method reads the first GATrackCtlDSP module and copies the `volumeValues[]`, `panValues[]`, and `muteValues[]` arrays into a `TrackCtlOutputs` struct along with connection flags. The host's `process()` method then applies these values to the patch library:
   - If the volume input is connected and the volume value for track `i` is >= 0.0, it calls `patchLibrary.setVolume(i, value)`.
   - If the pan input is connected and the pan value for track `i` is >= 0.0, it calls `patchLibrary.setPan(i, value)`.
   - If the mute input is connected, it calls `patchLibrary.setMuted(i, value)` for each track.

### Selective Override Behavior

The sentinel value system (-1.0) enables selective control. You can connect a cable to the VOL input to automate volume while leaving PAN and MUTE disconnected to preserve their manually-set values in the patch library. Each parameter is independent -- connecting one does not affect the others.

### Single Instance Limitation

The host reads only the first GATrackCtlDSP module found in the Sequencer Control patch. Placing multiple GATrackCtl modules in the same patch will not combine their values; only the first one discovered during patch building will be used.

### Stateless Design

The module has no configurable parameters and no persistent state beyond the current sample's values. The `volumeValues[]`, `panValues[]`, and `muteValues[]` arrays are overwritten every sample during `process()`. The `syncToDSP` and `syncFromDSP` methods are both no-ops because this is a pure input-only control module with no UI parameters to synchronize. The `reset()` method sets all volume and pan values to -1.0 (no override) and all mute values to false.

## Tips

- Place this module in the Sequencer Control global patch (not in regular step patches). It only functions when the host's Sequencer Control processing path discovers it.
- To automate volume fades across tracks, patch a polyphonic LFO or envelope signal into the VOL input. Each polyphonic channel controls a different track's volume independently.
- For a simple global volume control, connect a single mono cable to the VOL input. The mono value automatically broadcasts to all 16 tracks, giving you a master volume fader.
- Use the PAN input with a slow polyphonic LFO to create automated stereo movement across the mix. A 5V constant at the PAN input places all tracks at center.
- The MUTE input responds to gate signals, making it useful with pattern generators, sequencers, or logic modules to create rhythmic muting patterns. Any signal above 0.5V will mute the corresponding track.
- Combine with GAPatGen or GASequencer modules in the Sequencer Control patch to build programmatic mute patterns, volume automation, or pan sequences entirely within GrooveboxAdvanced.
- Since VOL and PAN use 0-10V scaling, standard VCV Rack CV sources work directly. A 10V signal represents maximum volume or hard-right pan; a 0V signal represents silence or hard-left pan.
- When the MUTE input is disconnected, all tracks default to unmuted. This differs from the VOL and PAN inputs, which use a -1.0 sentinel to mean "no override." Disconnecting the MUTE input will actively set all tracks to unmuted, overriding any manual mute states only while a cable was connected -- once disconnected, the mute override stops because `hasInput[2]` becomes false.
