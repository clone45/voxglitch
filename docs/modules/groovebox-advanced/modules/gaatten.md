# GAAtten

An attenuverter utility module that scales, inverts, and offsets signals. It multiplies an input signal by an adjustable bipolar amount (-1 to +1), adds a DC offset, and provides both the normal and phase-inverted results. The amount parameter can be modulated by an external CV signal for voltage-controlled amplitude and polarity.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | The primary signal input to be attenuated, amplified, or inverted. Accepts any signal type (audio, CV, gates, etc.). |
| 1 | CV | Control (CV) | Modulation input for the AMT parameter. A +/-5V signal maps to +/-1.0 and is added to the base AMT knob value. The combined effective amount is clamped to the -1.0 to +1.0 range. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | The processed output signal: `(IN * effectiveAmount) + offset`. |
| 3 | INV | Audio | The phase-inverted output: the negative of the OUT signal. When OUT produces +3V, INV produces -3V. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| AMT | Knob | -1.0 - 1.0 | 1.0 | Attenuation/inversion amount. At +1.0 the input passes through at unity gain. At 0.0 the input is fully attenuated (silent). At -1.0 the input passes through at unity gain but phase-inverted. Intermediate values scale proportionally. This value is summed with the scaled CV input. |
| OFS | Knob | -5.0 - 5.0 | 0.0 | DC offset in volts, added to the signal after attenuation. At 0.0 no offset is applied. Positive values shift the output upward; negative values shift it downward. |

## Details

### Signal Flow

The processing is straightforward and stateless (no internal memory or filtering):

1. **CV Scaling**: The CV input is divided by 5.0 to convert from VCV Rack's standard +/-5V range to a normalized +/-1.0 range.

2. **Effective Amount Calculation**: The scaled CV value is added to the base AMT knob value. The result is clamped to the -1.0 to +1.0 range, preventing the attenuation factor from exceeding unity gain even with extreme CV modulation.

3. **Attenuation and Offset**: The input signal is multiplied by the effective amount, then the OFS value is added: `output = (input * effectiveAmount) + offset`.

4. **Inverted Output**: The INV output is simply the negation of the OUT signal: `inverted = -output`. Note that the offset is also inverted, so a positive DC offset on OUT becomes a negative DC offset on INV.

### CV Modulation Behavior

The CV input acts as an additive modulator on the AMT knob. For example:

- With AMT set to 0.5 and a +2.5V CV signal (+0.5 after scaling), the effective amount becomes 1.0 (unity).
- With AMT set to 0.0 and a full +/-5V LFO on CV, the effective amount sweeps from -1.0 to +1.0, creating a ring-modulation-like tremolo effect.
- Because the effective amount is clamped, AMT at 1.0 with a positive CV signal still caps at 1.0 -- no signal boost beyond unity is possible.

## Tips

- Use as a simple VCA by setting AMT to 0.0 and patching an envelope or LFO into the CV input. The signal will scale from silence to full volume as the CV sweeps from 0V to +5V.
- Patch a constant voltage (such as from a CV source set to 10V) into IN with AMT at 0.5 and OFS at 0.0 to create an adjustable DC voltage source, then use the AMT knob or CV input to dial in the exact voltage you need.
- Use the INV output alongside OUT to create a pair of complementary signals for stereo width, balanced modulation, or differential patching.
- The OFS knob is useful for biasing CV signals. For example, shift a bipolar +/-5V LFO into a unipolar 0-5V range by setting OFS to 2.5 and AMT to 0.5.
- Set AMT to -1.0 to get a simple signal inverter: the OUT port produces the negated input and the INV port passes the original signal unchanged.
- Chain multiple GAAtten modules to build more complex signal math: one for scaling, another for offset, feeding into a mixer or modulation destination.
