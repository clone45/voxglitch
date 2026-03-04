# GAMixer4

A 4-input audio mixer that sums up to four signals with individual level controls and a master output level. It belongs to the Mixing category and is useful for combining multiple audio or CV sources into a single signal within a GrooveboxAdvanced patch.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN 1 | Audio | First audio input to be mixed. The signal is scaled from VCV Rack's +/-5V standard to a normalized +/-1V internal range before level scaling is applied. |
| 1 | IN 2 | Audio | Second audio input to be mixed. Processed identically to IN 1. |
| 2 | IN 3 | Audio | Third audio input to be mixed. Processed identically to IN 1. |
| 3 | IN 4 | Audio | Fourth audio input to be mixed. Processed identically to IN 1. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 4 | OUT | Audio | The summed and master-scaled output signal, converted back to VCV Rack's +/-5V range. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| IN 1 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 1. At 1.0 the input passes at full level; at 0.0 the input is fully silenced. |
| IN 2 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 2. Behaves identically to IN 1. |
| IN 3 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 3. Behaves identically to IN 1. |
| IN 4 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 4. Behaves identically to IN 1. |
| MSTR | Knob | 0.0 - 1.0 | 1.0 | Master output level applied after summing all inputs. At 1.0 the summed signal passes at full level; at 0.0 the output is fully silenced. |

## Details

### Signal Flow

The mixer is stateless -- it performs a simple sum-and-scale operation each sample with no internal memory, filtering, or feedback.

1. **Input Normalization**: Each input is divided by 5.0 to convert from VCV Rack's standard +/-5V range to a +/-1.0 internal range.

2. **Per-Channel Level**: Each normalized input is multiplied by its corresponding level knob (IN 1 through IN 4). With all levels at their default of 1.0, the normalized inputs pass through unchanged.

3. **Summation**: The four level-scaled signals are summed together. Because each input is normalized to +/-1.0 before summing, four full-amplitude inputs at unity level will produce a sum of up to +/-4.0.

4. **Master Level**: The summed signal is multiplied by the MSTR knob value.

5. **Output Scaling**: The result is multiplied by 5.0 to convert back to VCV Rack's +/-5V range. The final output formula is: `OUT = ((in1/5 * level1) + (in2/5 * level2) + (in3/5 * level3) + (in4/5 * level4)) * masterLevel * 5`.

### Gain Staging

With all four inputs driven at +/-5V and all levels at 1.0 (including master), the maximum output amplitude is +/-20V (4 channels * 5V). There is no built-in clipping or saturation. If you need to keep the output within +/-5V when mixing multiple hot signals, reduce the individual channel levels or the master level accordingly. For four equal-level sources, setting each channel level to 0.25 (or the master to 0.25) will keep the output within +/-5V.

### Unconnected Inputs

Inputs that have no cable connected will read 0V, so they contribute nothing to the sum. You do not need to turn down the level knob on unused channels.

## Tips

- Use the individual level knobs to set a rough balance between sources, then use the MSTR knob to control the overall volume of the submix without disturbing the relative levels.
- For submixing drums, patch a kick drum into IN 1, a snare into IN 2, and hi-hats into IN 3, then route the OUT to the patch's output module. Adjust individual levels for the drum balance and use MSTR as a drum bus fader.
- If you need more than four inputs, chain two GAMixer4 modules: route the OUT of one mixer into an input of another mixer.
- When using the mixer for CV signals rather than audio, remember that all signals pass through the same normalization and scaling. A 0-5V envelope will behave as expected; the level knob acts as a simple attenuator on its contribution to the sum.
- For a quick mute of a single source, turn its level knob to 0.0. For muting the entire mix, turn the MSTR knob to 0.0.
- If you only need two or three inputs, the GAMixer (3-input variant) is more compact. If you need more channels, consider the GAMixer6 or GAMixer8 variants.
