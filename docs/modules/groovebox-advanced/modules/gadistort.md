# GADistort

A waveshaping distortion module that uses a tanh (hyperbolic tangent) saturation curve to add harmonic content and overdrive to audio signals. At low drive settings it acts as a subtle saturator that gently rounds peaks, while at high drive settings it aggressively clips the signal toward a square wave, producing rich odd-harmonic distortion.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | Audio input signal to be distorted. The signal is multiplied by the drive amount before being passed through the tanh waveshaper. |
| 1 | DRV | Control (CV) | Drive CV modulation. A unipolar 0-10V signal that adds to the DRIVE knob value. The CV is scaled so that 10V adds 0.5 to the drive parameter (before the drive-to-multiplier mapping). The combined drive value is clamped to the 0.0-1.0 range. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | Mono audio output. The waveshaped signal, which is bounded to the -1.0 to +1.0 range due to the tanh saturation curve. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| DRIVE | Knob | 0.0 - 1.0 | 0.5 | Drive amount. Controls the intensity of the distortion by setting the gain multiplier applied to the input signal before waveshaping. The knob value is mapped from the 0.0-1.0 range to a 1x-50x multiplier. At 0.0 (1x multiplier), the signal passes through with minimal coloration. At 0.5 (approximately 25.5x), moderate distortion and compression are applied. At 1.0 (50x), the signal is hard-clipped into a near-square wave shape. |

## Details

### Signal Flow

1. **Input**: The audio signal is read from the IN port.

2. **Drive Calculation**: The effective drive is computed by combining the DRIVE knob value with the DRV CV input. The CV follows VCV Rack's 0-10V unipolar standard: the voltage is divided by 10, then multiplied by 0.5 to produce an additive offset in the 0.0-0.5 range. This offset is added to the knob value, and the result is clamped to 0.0-1.0. The final drive value is then mapped to a gain multiplier using the formula `driveAmount = drive * 49 + 1`, producing a multiplier that ranges linearly from 1x to 50x.

3. **Waveshaping**: The input signal is multiplied by the drive amount and then halved (multiplied by 0.5) before being passed through a fast tanh approximation. The halving shifts the operating point so that the waveshaper's soft-clipping region is centered appropriately for the drive range. The tanh function used is a Pade approximation: `tanh(x) = x * (27 + x^2) / (27 + 9 * x^2)`, which is accurate to approximately 0.001 within the -3 to +3 range and saturates to exactly -1 or +1 outside that range.

4. **Output**: The waveshaped signal is written directly to the OUT port. Because tanh asymptotically approaches +/-1, the output is inherently bounded and cannot clip beyond the -1.0 to +1.0 range regardless of input level or drive setting.

### Harmonic Character

Tanh waveshaping is a symmetric, odd-function distortion curve. This means it produces primarily odd harmonics (3rd, 5th, 7th, etc.) when applied to a sine wave, similar in character to tube amplifier saturation. At low drive values the curve is nearly linear and adds only subtle harmonic content. As drive increases, the curve flattens near +/-1, progressively squaring off the waveform peaks and adding increasingly strong higher-order odd harmonics.

### CV Modulation Behavior

The DRV CV input is additive and unipolar. At 0V the CV has no effect; at 10V it adds 0.5 to the drive parameter (half of the full knob range). This means:
- With the DRIVE knob at 0.0, a 10V CV brings the effective drive to 0.5 (approximately 25.5x gain).
- With the DRIVE knob at 0.5, a 10V CV brings the effective drive to 1.0 (50x gain, maximum distortion).
- With the DRIVE knob already at 1.0, the CV has no further effect because the combined value is clamped to 1.0.

Negative voltages at the DRV input will subtract from the drive value, allowing the CV to reduce distortion below the knob setting. The final value is always clamped to 0.0-1.0.

### Output Level

Because tanh saturates at +/-1.0, the output level is compressed relative to the input as drive increases. A clean signal at, for example, +/-5V will be reduced to approximately +/-1V at high drive settings. This inherent gain reduction means GADistort also functions as a soft limiter. If unity output level is needed after distortion, follow it with a GAAtten or GAScale module to restore gain.

## Tips

- For subtle tape-style saturation, keep the DRIVE knob low (0.1-0.3). This rounds off transient peaks without dramatically changing the tonal character, adding a slight warmth to the signal.
- For aggressive fuzz-like distortion, set the DRIVE knob above 0.7. At these settings, sine waves are transformed into near-square waves with a rich odd-harmonic spectrum.
- Use an envelope or LFO on the DRV CV input to create dynamic distortion that changes over the course of a note. For example, patching an envelope with a fast attack and moderate decay into DRV creates a distortion "bite" on the note attack that fades to a cleaner sustain.
- Place a GAFilter after GADistort to tame the harsh high-frequency harmonics introduced by heavy distortion. A low-pass filter at moderate cutoff frequencies simulates the speaker cabinet filtering found in guitar amplifier chains.
- Because the output is bounded to +/-1.0, GADistort can be used as a limiter to prevent clipping in downstream modules. Feed a signal with unpredictable peaks into it at low-to-moderate drive settings to keep levels controlled.
- Chain two GADistort modules for more extreme waveshaping. The first stage compresses and saturates the signal, and the second stage reshapes the already-distorted waveform, producing denser harmonic textures.
- When using GADistort on a mixed signal, be aware that the tanh curve introduces intermodulation distortion between frequency components. For cleaner results, distort individual oscillator signals before mixing rather than distorting the mix.
