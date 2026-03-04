# GAWavefolder

A multi-stage wavefolder that folds an audio signal back on itself when it exceeds internal bounds, creating harmonically rich timbres characteristic of west coast synthesis. The fold amount controls how many times the signal reflects, adding dense harmonic content, while the symmetry parameter introduces a DC offset before folding to produce asymmetric waveforms with even-harmonic content.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | Audio input signal to be folded. The signal follows VCV Rack's +/-5V audio standard and is internally scaled to a +/-1V range before processing. |
| 1 | FOLD | Control (CV) | Fold amount CV modulation. A unipolar 0-10V signal that adds to the FOLD knob value. The CV is divided by 10 to produce a 0.0-1.0 offset, which is added to the knob value. The combined fold amount is clamped to the 0.0-1.0 range. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | Mono audio output. The folded signal, scaled back to VCV Rack's +/-5V audio standard and hard-clamped to that range for safety. A final soft saturation stage (fast tanh) smooths any remaining sharp edges before output. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| FOLD | Knob | 0.0 - 1.0 | 0.5 | Fold amount. Controls the gain multiplier applied to the input signal before it enters the folding stage. The knob value is mapped from the 0.0-1.0 range to a 1x-10x gain multiplier using the formula `gain = fold * 9 + 1`. At 0.0 (1x gain), the signal passes through unfolded. At 0.5 (5.5x gain), moderate folding occurs. At 1.0 (10x gain), the signal folds multiple times, producing dense harmonic content. |
| SYM | Knob | 0.0 - 1.0 | 0.5 | Symmetry. Introduces a DC offset to the signal before it enters the folding stage, which causes the positive and negative halves of the waveform to fold differently. The knob value is mapped from 0.0-1.0 to a -1.0 to +1.0 DC offset using the formula `offset = (symmetry - 0.5) * 2`. At the default value of 0.5, the offset is zero and folding is symmetric. Values below 0.5 shift the signal negative before folding, and values above 0.5 shift it positive. After folding, half of the DC offset is subtracted back out, leaving some intentional asymmetry in the output. |

## Details

### Signal Flow

1. **Input Scaling**: The audio signal from the IN port is divided by 5.0 to convert from VCV Rack's +/-5V standard to an internal +/-1V working range.

2. **Fold Amount Calculation**: The effective fold amount is computed by adding the FOLD knob value and the FOLD CV input (divided by 10 to normalize 0-10V to 0.0-1.0). The result is clamped to the 0.0-1.0 range, then mapped to a gain multiplier: `gain = effectiveFold * 9.0 + 1.0`, producing a range of 1x to 10x.

3. **Gain Stage**: The scaled input signal is multiplied by the gain value. This amplification is what causes the signal to exceed the +/-1.0 folding thresholds, and the amount of gain directly determines how many folds will occur.

4. **DC Offset (Symmetry)**: A DC offset derived from the SYM knob is added to the gained signal. The offset maps the 0.0-1.0 knob range to -1.0 to +1.0. At the default center position (0.5), the offset is zero.

5. **Multi-Stage Folding**: The signal passes through a 4-iteration folding loop. In each iteration, if the signal exceeds +1.0, it is reflected back via `signal = 2.0 - signal`. If the signal falls below -1.0, it is reflected back via `signal = -2.0 - signal`. Each reflection folds the waveform at the boundary, and the 4 iterations allow for multiple successive folds when the gain is high enough to push the signal well beyond the thresholds.

6. **Partial DC Offset Removal**: After folding, half of the original DC offset is subtracted from the signal (`signal -= dcOffset * 0.5`). This partial removal keeps some of the asymmetry introduced by the SYM parameter while preventing excessive DC content in the output.

7. **Soft Saturation**: The folded signal is passed through a fast tanh approximation (Pade approximation: `tanh(x) = x * (27 + x^2) / (27 + 9 * x^2)`). This smooths any sharp corners left by the folding reflections, preventing harsh aliasing artifacts and ensuring the signal remains bounded.

8. **Output Scaling and Clamping**: The signal is multiplied by 5.0 to return to VCV Rack's +/-5V audio standard, then hard-clamped to the +/-5V range as a safety measure.

### Harmonic Character

At low fold amounts (gain near 1x), the signal passes through mostly unchanged, with only the tanh soft saturation adding subtle coloration. As the fold amount increases, the waveform begins to fold once, then twice, then multiple times. Each additional fold introduces new harmonic content. Because the folding is a nonlinear reflection (not clipping), the harmonics are distributed differently than with standard distortion -- the spectrum tends to be dense and metallic, with both odd and even harmonics present depending on the symmetry setting.

When the SYM knob is at its center (0.5), the folding is symmetric, which primarily generates odd harmonics (similar to clipping or traditional wavefolding). Moving the SYM knob away from center breaks this symmetry, introducing even harmonics into the spectrum. This makes the SYM parameter useful for shifting the tonal character from hollow and nasal (symmetric, odd harmonics) toward brighter and more complex (asymmetric, mixed harmonics).

### CV Modulation Behavior

The FOLD CV input is additive and uses a 0-10V unipolar convention. At 0V the CV has no effect; at 10V it adds 1.0 to the fold parameter (the full knob range). This means:
- With the FOLD knob at 0.0, a 10V CV brings the effective fold to 1.0 (maximum folding, 10x gain).
- With the FOLD knob at 0.5, a 10V CV brings the effective fold to 1.0 (clamped at maximum).
- With the FOLD knob at 0.0, a 5V CV brings the effective fold to 0.5 (moderate folding, 5.5x gain).

Negative voltages at the FOLD input will subtract from the fold amount, allowing CV to reduce folding below the knob setting. The final value is always clamped to 0.0-1.0.

### Output Level

The output level varies with fold amount. At low fold amounts the signal passes through at roughly unity gain (the tanh stage is nearly linear for small signals). At higher fold amounts, the waveform is folded back into the +/-1 range and then scaled to +/-5V, so the peak output remains close to +/-5V, but the RMS level and waveform shape change significantly.

## Tips

- For classic west coast synthesis tones, patch a sine or triangle wave into the IN port and sweep the FOLD knob from 0.0 to 1.0. The wavefolder transforms a simple waveform into increasingly complex timbres without the harsh quality of distortion or clipping.
- Use an envelope on the FOLD CV input to create evolving timbres that change over the course of a note. A slow attack on the fold amount produces a sound that starts pure and gradually becomes more harmonically dense, similar to a plucked string's initial brightness fading to a mellow sustain, but in reverse.
- Experiment with the SYM knob to add even harmonics. At the center position the sound is hollow and vocal-like; moving SYM toward the extremes adds brightness and complexity. Small deviations from center (0.4 or 0.6) produce subtle tonal shifts, while extreme settings (0.0 or 1.0) create aggressive asymmetric folding.
- For metallic, bell-like textures, feed a sine wave with a high fold amount (0.8-1.0). The dense harmonic series produced by multiple folds has a quality reminiscent of FM synthesis at high modulation indices.
- Place a GAFilter after GAWavefolder to sculpt the harmonic content. A low-pass filter with moderate resonance after heavy folding can tame the upper harmonics while emphasizing a particular spectral peak, producing vowel-like or resonant tones.
- Modulate the SYM parameter with a slow LFO for shifting tonal color over time. Because symmetry changes the balance of odd and even harmonics, this creates a slow spectral animation that adds movement to sustained sounds.
- Chain GAWavefolder with GADistort for extreme waveshaping. The wavefolder creates complex harmonic content through reflection, and the distortion module further saturates and compresses the result, producing thick, aggressive textures suitable for industrial or noise music.
- At low FOLD settings (0.1-0.2), GAWavefolder functions as a subtle saturator similar to tape or tube warmth, adding gentle harmonic coloration without dramatically altering the waveform shape.
