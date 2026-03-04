# GAOperator

A full-featured FM/PM operator module modeled after the Yamaha DX7 architecture. GAOperator combines an oscillator with configurable frequency ratio, detune, feedback, modulation index, and key-scaling into a single module. It supports both true phase modulation (the method actually used by the DX7, despite its "FM" branding) and classic frequency modulation. With six waveform shapes, DX7-style exponential feedback scaling, and anti-aliasing key-scaling, GAOperator is the primary building block for multi-operator FM synthesis patches.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | FM | Audio | Modulation input for FM or PM synthesis. In PM mode (default), this signal directly offsets the oscillator's phase lookup, scaled by the modulation index. In FM mode, this signal modulates the phase increment, affecting the instantaneous frequency. The combined modulation signal (FM input plus feedback) is clamped to +/-10V before processing. |
| 1 | FREQ | Control (CV) | Exponential frequency CV input following the 1V/oct standard. Applied after the base frequency calculation (ratio or fixed). A +1V signal doubles the frequency; +2V quadruples it. Clamped to +/-10V before the exponential conversion. |
| 2 | SYNC | Trigger | Hard sync input. On a rising edge (crossing above 0.1V), the oscillator phase resets to the PHASE offset value. Use this for phase-locked sync effects or to retrigger the oscillator at precise moments. |
| 3 | LVL | Control (CV) | Level CV input using VCV Rack 0-10V unipolar standard. When connected, the input voltage is divided by 5V and multiplied with the LEVEL knob value: 0V = mute, 5V = unity (no change), 10V = 2x boost. Negative values are clamped to zero. When disconnected, only the LEVEL knob value is used. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 4 | OUT | Audio | Audio output scaled to VCV Rack standard levels (+/-5V). The oscillator waveform (range -1 to +1) is multiplied by the effective level, then scaled by 5.0. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| RATIO | Knob | 0.25 - 32.0 | 1.0 | Frequency ratio multiplier. The base frequency (from the patch note) is multiplied by this ratio. Common FM ratios: 1.0 (unison), 2.0 (octave up), 0.5 (octave down), 3.0 (octave + fifth). Non-integer ratios produce inharmonic, bell-like or metallic timbres. Ignored when FREQ mode is set to Fixed. |
| WAVE | Dropdown | Sin / Saw / Sqr / Tri / Abs / Hlf | Sin | Selects the oscillator waveform shape. Sin uses a 1024-entry lookup table with linear interpolation. Saw is a naive linear ramp. Sqr is a 50% duty cycle hard square. Tri is a piecewise linear triangle. Abs is full-wave rectified sine (abs(sin) scaled to bipolar -1 to +1). Hlf is half-wave rectified sine (positive sine lobes only, negative half is zero). |
| LEVEL | Knob | 0.0 - 1.0 | 1.0 | Output amplitude scaling applied before the final 5V normalization. Further modulated by the LVL CV input when connected. |
| MOD IX | Knob | 0.0 - 10.0 | 1.0 | Modulation index controlling the depth of FM/PM modulation from the FM input and self-feedback. Internally converted from radians to cycles (divided by 2*pi) before scaling the modulation signal. An index of 1.0 corresponds to 1 radian of peak phase deviation in PM mode. Higher values produce more sidebands and a brighter, more complex timbre. Subject to key-scaling reduction when KEYSCL is active. |
| DETUNE | Knob | -100.0 - 100.0 cents | 0.0 | Fine-tuning offset in cents (hundredths of a semitone). Applied as a multiplicative frequency adjustment using the formula 2^(cents/1200). Full range spans one semitone in either direction. |
| FDBK | Knob | 0.0 - 1.0 | 0.0 | Self-feedback amount using DX7-style exponential scaling. The operator's own output from the previous two samples is averaged (anti-hunting filter from the Tomisawa patent US4249447A), then scaled by 2^(7*feedback - 7). This exponential curve means the feedback doubles with each ~0.143 increase in knob position. At 0.0, feedback is effectively off; at 1.0, the scaling factor reaches 1.0 (maximum). Self-feedback on a sine operator produces increasingly saw-like timbres, eventually reaching noise at high settings. |
| PHASE | Knob | 0.0 - 1.0 | 0.0 | Initial phase offset as a fraction of one full cycle. The oscillator resets to this phase value on sync trigger and on step reset. 0.5 inverts the starting polarity of the waveform. |
| KEYSCL | Knob | 0.0 - 1.0 | 0.0 | Key-scaling amount that automatically reduces the effective modulation index at higher frequencies to prevent aliasing. At 0.0, no scaling is applied and the full MOD IX value is used at all pitches. At 1.0, the modulation index is clamped to keep FM sidebands below the Nyquist frequency (with a 20% safety margin). Intermediate values blend between the user-set index and the safe limit. |
| MODE | Switch | FM / PM | PM | Selects the modulation mode. PM (phase modulation) adds the modulation signal directly to the phase lookup, which is the method used by the DX7 and produces stable pitch regardless of modulation depth. FM (frequency modulation) adds the modulation signal to the phase increment, which slightly drifts the pitch under deep modulation. PM is recommended for most use cases. |
| FREQ | Switch | Ratio / Fixed | Ratio | Selects the frequency mode. In Ratio mode, the operator frequency is the patch base frequency multiplied by the RATIO knob. In Fixed mode, the operator uses an absolute frequency in Hz regardless of the played note, useful for fixed-pitch components like bells, noise textures, or percussion elements. |

## Details

### Signal Flow

The DSP processing chain operates in this order:

1. **Sync detection**: The SYNC input is checked for a rising edge (crossing above 0.1V). If detected, the oscillator phase is reset to the PHASE offset value.

2. **Base frequency calculation**: In Ratio mode, the base frequency (set by the patch's current note) is multiplied by the RATIO knob value. In Fixed mode, the fixed frequency (440 Hz) is used directly.

3. **Detune**: If the DETUNE knob is non-zero, a multiplicative frequency adjustment is applied using 2^(cents/1200). This allows fine pitch adjustments of up to +/-1 semitone.

4. **Frequency CV**: The FREQ CV input applies exponential (1V/oct) scaling using `exp2_taylor5`. The CV is clamped to +/-10V before conversion.

5. **Frequency clamping**: The final frequency is clamped to the range 0.01--20000 Hz to prevent runaway oscillation or sub-audio drift.

6. **Feedback calculation**: The average of the two most recent output samples is computed (DX7-style anti-hunting filter). This averaged value is scaled by the exponential feedback curve: 2^(7 * FDBK - 7). The scaled feedback is summed with the FM input to produce the total modulation signal, which is then clamped to +/-10V.

7. **Key-scaling**: If KEYSCL is greater than zero, the effective modulation index is reduced based on the current frequency relative to the Nyquist limit. The module estimates the maximum safe index that keeps sidebands below Nyquist (with an 80% safety margin) and blends between the user-set index and this safe limit according to the KEYSCL knob position.

8. **Phase accumulation and modulation**: The phase advances by `frequency * sampleTime` each sample and wraps to [0, 1). In PM mode, the modulation signal (FM input + feedback, scaled by index-in-cycles) is added to the phase lookup position. In FM mode, the modulation signal adjusts the phase increment, with clamping to prevent aliasing from excessively large increments.

9. **Waveform generation**: The phase (or modulated phase in PM mode) is converted to an output sample according to the selected waveform shape.

10. **Level and output**: The waveform sample is multiplied by the effective level (LEVEL knob times LVL CV scaling if connected), stored in the feedback history buffer, and then scaled by 5.0 to produce VCV Rack standard audio levels (+/-5V at full level).

### Waveform Details

- **Sin**: 1024-point lookup table with linear interpolation. Clean and alias-free. The standard choice for FM synthesis.
- **Saw**: Naive `2 * phase - 1` ramp. Not band-limited; aliasing is managed by the GAProcessor's 4x oversampling.
- **Sqr**: Hard threshold at 50% duty cycle. Not band-limited; relies on oversampling for anti-aliasing.
- **Tri**: Piecewise linear with four segments per cycle. Naturally weak upper harmonics minimize aliasing.
- **Abs**: Full-wave rectified sine: `abs(sin(phase)) * 2 - 1`, mapped to bipolar range. Produces a distinctive even-harmonic timbre at double the fundamental frequency.
- **Hlf**: Half-wave rectified sine: outputs the positive sine lobes, zeros out the negative half. Produces asymmetric distortion harmonics with a strong fundamental.

### Feedback Behavior

The self-feedback implementation follows the Yamaha DX7 design (Tomisawa patent). Two key aspects:

- **Anti-hunting filter**: Instead of feeding back only the previous sample, the average of the previous two samples is used. This prevents the oscillatory instability ("hunting") that occurs with single-sample feedback at high feedback levels.
- **Exponential scaling**: The DX7 used 8 discrete feedback levels (0--7), each doubling the feedback amount. This module maps the continuous 0--1 knob range to the same exponential curve using 2^(7*x - 7). Low knob values produce subtle warmth; values above 0.7 introduce significant harmonic content; values near 1.0 approach noise-like output.

### Anti-Aliasing

PolyBLEP anti-aliasing is intentionally not used in this module because it is incompatible with phase modulation. In PM/FM synthesis, aliasing arises from sideband generation (carrier +/- N * modulator), not from waveform discontinuities. The GAProcessor handles this through 4x oversampling of the entire FM algorithm.

### Phase Reset

The oscillator phase resets to the PHASE offset value when `reset()` is called (at the start of each new sequencer step) and on rising edges at the SYNC input.

## Tips

- For classic DX7-style sounds, keep MODE set to PM and use Sin waveforms. Connect one GAOperator's OUT to another GAOperator's FM input to build 2-operator, 3-operator, or larger FM algorithms.
- Start with low MOD IX values (0.5--2.0) and increase gradually. The modulation index directly controls timbral brightness -- animate it with an envelope connected through a VCA for evolving timbres that brighten on attack and darken on release.
- Use integer RATIO values (1, 2, 3, 4...) for harmonic timbres. Non-integer ratios (1.41, 2.76, 7.13) produce inharmonic, metallic, or bell-like sounds. The ratio 1.4142 (square root of 2) is a classic choice for bell tones.
- Self-feedback (FDBK) on a sine operator progressively transforms the timbre from pure sine through saw-like harmonics to noise. Values around 0.4--0.6 add subtle warmth and presence without overwhelming the sound.
- Use the SYNC input with a clock or trigger source to phase-lock multiple operators together, ensuring consistent attack transients. This is especially useful for percussive FM sounds.
- Enable KEYSCL when using high modulation indices across a wide pitch range. Without key-scaling, high notes can become harsh and aliased because the FM sidebands exceed the Nyquist frequency. Key-scaling automatically tames the upper register.
- For fixed-pitch percussion (hi-hats, bells, metallic effects), set one or more operators to Fixed mode. This keeps their frequency constant regardless of the played note, while the ratio-mode operators track pitch normally.
- The DETUNE parameter is useful for creating subtle chorus-like thickness when two operators share the same ratio but are detuned by 5--15 cents in opposite directions.
- The Abs and Hlf waveforms add even-harmonic content that is difficult to achieve with standard sine FM. Try them as modulators for organ-like or distorted timbres.
- Connect an envelope to the LVL input to create carrier-style amplitude shaping, or use it on a modulator operator to dynamically control FM depth over the note's duration.
