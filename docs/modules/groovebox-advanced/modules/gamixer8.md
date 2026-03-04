# GAMixer8

An 8-input audio mixer that sums up to eight signals into a single mono output. Each input has an independent level knob, and a master level knob scales the combined sum. The mixer is stateless and operates purely as a weighted summing amplifier with no filtering, feedback, or memory between samples.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN 1 | Audio | First audio input. The signal is scaled from VCV Rack +/-5V standard to +/-1V internal range, then multiplied by the IN 1 level knob before summing. |
| 1 | IN 2 | Audio | Second audio input. Scaled and attenuated identically to IN 1, using the IN 2 level knob. |
| 2 | IN 3 | Audio | Third audio input. Scaled and attenuated identically to IN 1, using the IN 3 level knob. |
| 3 | IN 4 | Audio | Fourth audio input. Scaled and attenuated identically to IN 1, using the IN 4 level knob. |
| 4 | IN 5 | Audio | Fifth audio input. Scaled and attenuated identically to IN 1, using the IN 5 level knob. |
| 5 | IN 6 | Audio | Sixth audio input. Scaled and attenuated identically to IN 1, using the IN 6 level knob. |
| 6 | IN 7 | Audio | Seventh audio input. Scaled and attenuated identically to IN 1, using the IN 7 level knob. |
| 7 | IN 8 | Audio | Eighth audio input. Scaled and attenuated identically to IN 1, using the IN 8 level knob. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 8 | OUT | Audio | Mono summed output. The weighted sum of all eight inputs is multiplied by the master level, then scaled back to VCV Rack +/-5V standard. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| IN 1 | Knob | 0.0 - 1.0 | 1.0 | Level for input channel 1. At 0.0 the channel is fully muted; at 1.0 the channel passes at unity gain (after the internal +/-5V to +/-1V scaling). |
| IN 2 | Knob | 0.0 - 1.0 | 1.0 | Level for input channel 2. Behaves identically to IN 1. |
| IN 3 | Knob | 0.0 - 1.0 | 1.0 | Level for input channel 3. Behaves identically to IN 1. |
| IN 4 | Knob | 0.0 - 1.0 | 1.0 | Level for input channel 4. Behaves identically to IN 1. |
| IN 5 | Knob | 0.0 - 1.0 | 1.0 | Level for input channel 5. Behaves identically to IN 1. |
| IN 6 | Knob | 0.0 - 1.0 | 1.0 | Level for input channel 6. Behaves identically to IN 1. |
| IN 7 | Knob | 0.0 - 1.0 | 1.0 | Level for input channel 7. Behaves identically to IN 1. |
| IN 8 | Knob | 0.0 - 1.0 | 1.0 | Level for input channel 8. Behaves identically to IN 1. |
| MSTR | Knob | 0.0 - 1.0 | 1.0 | Master output level. Scales the summed signal of all eight channels before it is converted back to +/-5V. At 0.0 the output is silent; at 1.0 the full sum passes through. |

## Details

### Signal Flow

1. **Input Scaling**: Each of the eight input signals is divided by 5.0 to convert from VCV Rack's +/-5V audio standard to a +/-1.0 internal representation.

2. **Per-Channel Attenuation**: Each scaled input is multiplied by its corresponding level knob (0.0 to 1.0). Because the knob range is 0.0 to 1.0, each channel can only attenuate -- it cannot boost a signal beyond its original level.

3. **Summation**: All eight attenuated signals are added together. With all eight channels at unity and receiving full-scale +/-5V signals, the internal sum can reach up to +/-8.0.

4. **Master Level**: The sum is multiplied by the master level knob (0.0 to 1.0). This provides a single control point to attenuate the overall mix without adjusting individual channels.

5. **Output Scaling**: The result is multiplied by 5.0 to convert back to VCV Rack's +/-5V range. The output is written directly to the OUT port with no clamping or saturation, so summing many loud signals can produce output voltages exceeding +/-5V.

### Gain Structure

The effective gain from a single input port to the output port is: `(input / 5) * channelLevel * masterLevel * 5`, which simplifies to `input * channelLevel * masterLevel`. With both the channel and master knobs at 1.0 (their defaults), a single input passes through at unity gain. However, when multiple inputs are active simultaneously, their signals add constructively, and the output amplitude can exceed the input amplitude of any single channel. For example, eight identical +/-5V signals with all knobs at 1.0 will produce a +/-40V output, well above the typical +/-5V range. Use the channel level knobs or the master knob to keep the output within a reasonable range when mixing many active channels.

### Stateless Operation

The mixer performs no sample-to-sample state tracking. There are no filters, slew limiters, or envelope followers in the signal path. Each output sample is computed purely from the current input samples and the current knob positions. The `reset()` method is intentionally empty because there is no state to clear.

### Unconnected Inputs

Inputs that are not connected to any cable return 0V from `getInput()`, so they contribute nothing to the sum. There is no need to mute unused channels -- they are effectively silent by default.

## Tips

- Use GAMixer8 as the final mix bus for a patch. Route all oscillators, drum modules, and effect returns into the eight inputs and connect the OUT to a GAOutput module. Adjust the channel knobs to balance relative levels, and use the MSTR knob as a master fader.
- When mixing fewer than eight sources, leave the unused inputs unconnected. They contribute 0V and have no effect on the output. The unused channel level knobs can be left at their defaults.
- Because the mixer has no built-in saturation or limiting, summing many hot signals can produce output voltages that clip downstream modules. If the mix sounds harsh or distorted, reduce individual channel levels or turn down the MSTR knob. Alternatively, follow GAMixer8 with a GADistort at low drive settings to soft-clip the summed signal.
- To create a submix, use one GAMixer8 to combine a group of related signals (for example, all drum voices), then feed its output into one channel of a second GAMixer8 that handles the final mix. This gives you both per-voice and per-group level control.
- The channel knobs can be used as simple on/off mutes: set a knob to 0.0 to silence a channel and 1.0 to restore it. This is useful for quickly auditioning individual parts in a dense mix.
- Since GAMixer8 applies no coloration, it works equally well for mixing control voltages as it does for audio. Sum multiple LFOs or envelopes together to create complex modulation shapes, keeping in mind that the output may exceed the expected CV range and may need scaling afterward.
