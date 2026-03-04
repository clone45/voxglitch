# GAAmp

An amplifier and waveshaping module that boosts or attenuates signals with selectable saturation behavior at high levels. It can function as a clean gain stage, a soft-clipping saturator, a wavefolder, or a signal wrapper -- ranging from transparent level adjustment to aggressive harmonic distortion.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | Audio input signal to be amplified and shaped. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 1 | OUT | Audio | Processed audio output. The voltage range depends on the selected mode: Clean and Soft Clip output up to +/-10V, while Fold and Wrap output up to +/-5V. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| GAIN | Knob | 0.0 - 4.0 | 1.0 | Signal gain multiplier. At 1.0 the signal passes at unity. Values below 1.0 attenuate; values above 1.0 amplify. At maximum (4.0), the signal is quadrupled in amplitude before saturation is applied. |
| MODE | Dropdown | Clean / Soft Clip / Fold / Wrap | Soft Clip | Selects the saturation behavior applied after gain. See the Details section for a description of each mode. |

## Details

### Signal Flow

The module applies a simple two-stage process:

1. **Gain Stage**: The input signal is multiplied by the GAIN parameter. At unity (1.0) the signal is unchanged. At 0.0 the output is silent. At 4.0 a 5V input becomes 20V before saturation.

2. **Saturation/Shaping Stage**: The amplified signal is processed according to the selected MODE.

### Saturation Modes

- **Clean**: Hard clips the amplified signal at +/-10V using `rack::math::clamp`. Below the clipping threshold the signal is perfectly linear. At the threshold it clips abruptly, which can produce harsh harmonics if the signal is driven hard. This mode is useful when you want transparent gain with a safety limiter.

- **Soft Clip**: Applies hyperbolic tangent (tanh) saturation scaled so the output asymptotically approaches +/-10V. The formula is `10 * tanh(amplified * 0.1)`. At low levels this is nearly linear and transparent. As the signal is driven harder, the tanh curve compresses peaks smoothly, adding warm even-harmonic saturation without hard edges. This is the default mode.

- **Fold**: Wavefolding with a +/-5V boundary. When the amplified signal exceeds +5V or drops below -5V, it reflects back from the boundary. For example, a signal that reaches +7V is folded to +3V (reflected around +5V). Successive folds occur if the signal exceeds the boundary multiple times. This creates complex, bright, harmonically rich tones -- especially when driving the GAIN high. The output is always within +/-5V.

- **Wrap**: Wraps the signal around a +/-5V range using modular arithmetic. When the signal exceeds +5V it reappears at -5V and continues upward (and vice versa). This produces discontinuities that generate rich harmonic content with a character distinct from folding. The output is always within +/-5V.

### Stateless Processing

GAAmp is fully stateless -- it has no internal memory, envelopes, or filters. Each sample is processed independently based only on the current input and parameter values. This means there is no transient behavior, warmup time, or latency.

## Tips

- At GAIN 1.0 with Soft Clip mode, the module passes most signals transparently. Use it as a gentle safety limiter at the end of a chain to prevent clipping without harsh artifacts.
- Drive the GAIN above 2.0 in Fold mode to create bright, harmonically dense textures from simple waveforms. A sine wave into a wavefolder at high gain produces complex timbres reminiscent of West Coast synthesis.
- Use GAIN at values below 1.0 as a simple attenuator. In this range, all four modes behave identically since the signal never reaches the saturation threshold.
- Wrap mode at high gain values creates chaotic, glitchy tones that are useful for percussion or experimental sound design. Even small gain changes produce dramatic timbral shifts.
- Chain two GAAmp modules: use the first at moderate gain in Soft Clip mode for warmth, then a second in Clean mode at lower gain to trim the output level.
- Automate the GAIN parameter from another module's CV output to create dynamic distortion effects that respond to sequencer patterns or envelopes.
- When choosing between Fold and Wrap for distortion effects, note that Fold produces smoother waveform transitions (the signal reverses direction at boundaries), while Wrap produces hard discontinuities (the signal jumps from one boundary to the other). Fold tends to sound brighter and more musical; Wrap tends to sound harsher and more digital.
