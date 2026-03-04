# GAScale

A signal scaling utility module that multiplies an input signal by an adjustable factor (0 to 2) and adds a DC offset. Unlike the GAAtten module which offers bipolar attenuation from -1 to +1, GAScale provides unipolar scaling from 0 to 2, allowing the signal to be amplified up to double its original level. The scale amount can be modulated by an external CV signal for voltage-controlled gain.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | The primary signal input to be scaled. Accepts any signal type (audio, CV, gates, etc.). Operates at VCV Rack standard levels (typically +/-5V for audio). |
| 1 | CV | Control (CV) | Modulation input for the SCALE parameter. A +/-5V signal is scaled by 0.2 to produce a +/-1.0 modulation depth, which is added to the base SCALE knob value. The combined effective scale is clamped to the 0.0 to 2.0 range. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | The processed output signal: `(IN * effectiveScale) + offset`. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| SCALE | Knob | 0.0 - 2.0 | 1.0 | Scaling/multiply factor. At 1.0 the input passes through at unity gain. At 0.0 the input is fully attenuated (silent). At 2.0 the input is amplified to double its original level. This value is summed with the scaled CV input and the result is clamped to the 0.0 to 2.0 range. |
| OFFS | Knob | -1.0 - 1.0 | 0.0 | DC offset in volts, added to the signal after scaling. At 0.0 no offset is applied. Positive values shift the output upward; negative values shift it downward. |

## Details

### Signal Flow

The processing is stateless with no internal memory or filtering:

1. **CV Scaling**: The CV input is multiplied by 0.2 to convert from VCV Rack's standard +/-5V range to a +/-1.0 modulation depth. This means a full +5V CV signal adds 1.0 to the scale amount.

2. **Effective Scale Calculation**: The scaled CV value is added to the base SCALE knob value. The result is clamped to the 0.0 to 2.0 range, preventing negative scaling or excessive amplification.

3. **Scale and Offset**: The input signal is multiplied by the effective scale, then the OFFS value is added: `output = (input * effectiveScale) + offset`.

### CV Modulation Behavior

The CV input acts as an additive modulator on the SCALE knob. For example:

- With SCALE set to 1.0 and a +5V CV signal (+1.0 after scaling), the effective scale becomes 2.0 (double gain).
- With SCALE set to 1.0 and a -5V CV signal (-1.0 after scaling), the effective scale becomes 0.0 (silence).
- With SCALE set to 0.5 and a +/-5V LFO on CV, the effective scale sweeps from 0.0 to 1.5, creating a tremolo effect biased toward lower gain.
- Because the effective scale is clamped to the 0.0 to 2.0 range, it cannot go negative. This means GAScale cannot invert the signal phase -- use GAAtten if you need phase inversion.

### Comparison with GAAtten

GAScale and GAAtten are complementary:

- **GAAtten**: Bipolar amount (-1 to +1), includes phase inversion, provides an inverted output port, and has a wider offset range (-5 to +5V).
- **GAScale**: Unipolar factor (0 to 2), provides signal boost up to 2x, no phase inversion, single output, and a narrower offset range (-1 to +1V).

## Tips

- Use as a simple gain stage by setting SCALE to a value above 1.0 to boost quiet signals before feeding them into a mixer or output.
- Patch an envelope into the CV input with SCALE at 0.0 to create a VCA. As the envelope rises from 0V to +5V, the effective scale sweeps from 0.0 to 1.0, providing clean amplitude shaping.
- Use the OFFS parameter to add a small DC bias to a signal. This is useful for shifting a bipolar modulation source into a range that works better with a particular destination.
- For a ducking effect, patch an inverted envelope into CV with SCALE at 2.0. When the envelope fires, the effective scale drops, reducing the signal level temporarily.
- Chain GAScale after a GAFilter to compensate for volume loss caused by filtering: set SCALE to 1.5 or 2.0 to bring the filtered signal back up to a usable level.
- Combine with an LFO on the CV input and SCALE at 1.0 to create a tremolo effect that ranges between silence (0.0) and unity gain (2.0), centered around the original signal level.
