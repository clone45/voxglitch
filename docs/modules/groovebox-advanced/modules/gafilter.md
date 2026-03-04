# GAFilter

A state-variable filter (SVF) module that provides lowpass, highpass, and bandpass filtering with resonance control and CV-modulated cutoff frequency. The filter uses an efficient SVF topology that simultaneously computes all three filter responses from a single structure, allowing mode switching without discontinuities in the internal state.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | Audio input signal to be filtered. Expects standard VCV Rack audio levels (up to +/-5V). The signal is internally scaled to +/-1V for processing, then scaled back to +/-5V at the output. |
| 1 | FREQ | Control (CV) | Cutoff frequency CV modulation. A bipolar +/-10V signal is scaled to a +/-1.0 range and added directly to the CUTOFF knob value. For example, with CUTOFF at 0.5, a +5V CV shifts the effective cutoff to 0.75; a -5V CV shifts it to 0.25. The combined value is clamped to 0.0-1.0 before frequency mapping. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | Mono audio output carrying the filtered signal at standard VCV Rack levels (+/-5V). The active filter mode (LP, HP, or BP) determines which response is routed to this output. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| CUTOFF | Knob | 0.0 - 1.0 | 0.5 | Filter cutoff frequency, normalized. The value is mapped through an exponential lookup table to a frequency range of 20 Hz (at 0.0) to approximately 14,326 Hz (at 1.0). The default of 0.5 corresponds to roughly 511 Hz. The exponential mapping provides musically useful resolution across the entire range, with finer control in the low frequencies where pitch perception is more sensitive. |
| RES | Knob | 0.0 - 1.0 | 0.0 | Filter resonance. Controls the sharpness of the filter's response at the cutoff frequency. At 0.0 the filter has a gentle, broad rolloff. As resonance increases toward 1.0, a pronounced peak forms at the cutoff frequency, making the filter sound more aggressive and "vocal." Internally, the resonance is clamped to the range 0.01-0.99 to prevent instability. The resonance parameter controls the SVF damping factor as k = 2 - 2*resonance, meaning higher resonance values reduce damping and increase the peak amplitude. |
| MODE | Dropdown | LP, HP, BP | LP | Selects the filter response type. **LP** (Lowpass): passes frequencies below the cutoff and attenuates frequencies above it, producing a darker, warmer sound. **HP** (Highpass): passes frequencies above the cutoff and attenuates frequencies below it, thinning out low-end content. **BP** (Bandpass): passes a band of frequencies centered around the cutoff and attenuates both above and below, isolating a narrow frequency region. |

## Details

### Signal Flow

1. **Input Scaling**: The audio input is divided by 5.0 to normalize from VCV Rack's +/-5V standard to an internal +/-1V working range. The CV input is divided by 10.0 to normalize from +/-10V to +/-1.0.

2. **Coefficient Calculation**: The filter coefficients are recalculated only when the effective cutoff or resonance changes by more than 0.001. This dirty-checking optimization avoids redundant trigonometric computations on every sample when parameters are static.

3. **Cutoff Frequency Mapping**: The combined cutoff value (knob + CV, clamped to 0.0-1.0) is mapped through a 256-entry exponential lookup table with linear interpolation. The table provides a frequency range from 20 Hz to approximately 14,326 Hz, following the curve `20 * 750^(index/255)`. This exponential mapping means that equal knob movements produce equal perceived pitch changes across the range.

4. **Nyquist Protection**: The mapped frequency is clamped to 45% of the current sample rate to prevent the filter from becoming unstable near the Nyquist frequency. At 44.1 kHz, this limits the maximum effective cutoff to approximately 19,845 Hz.

5. **SVF Processing**: The module implements a linear state-variable filter (also known as the Chamberlin or trapezoidal integrator SVF). The core computation uses two integrator state variables (`ic1eq` and `ic2eq`) and produces three simultaneous outputs:
   - `v0`: Highpass output
   - `v1`: Bandpass output
   - `v2`: Lowpass output

   The SVF equations are:
   ```
   v3 = input - ic2eq
   v0 = a1 * v3 - ak * ic1eq
   v1 = a2 * v3 + a1 * ic1eq
   v2 = a3 * v3 + a2 * ic1eq + ic2eq
   ic1eq = 2 * v1 - ic1eq
   ic2eq = 2 * v2 - ic2eq
   ```

   Where the coefficients `a1`, `a2`, `a3`, and `ak` are derived from the cutoff frequency and resonance. The coefficient `g` is computed as `tan(pi * freq * sampleTime)` using a fast polynomial approximation, and `k = 2 - 2 * resonance` controls damping.

6. **Mode Selection**: The MODE dropdown selects which of the three SVF outputs is routed to the output port. Switching modes does not reset the filter state, so transitions between modes are smooth and free of clicks.

7. **Output Scaling**: The selected filter output is multiplied by 5.0 to scale back to VCV Rack's +/-5V audio standard.

8. **Safety**: After each sample, the integrator states are checked for NaN or infinity. If either state becomes non-finite (which can happen with extreme parameter changes or numerical drift), it is reset to 0.0 to prevent sustained noise or silence. Similarly, the `g` coefficient is clamped to the range 0.0-100.0 as a guard against extreme values from the tangent function.

### Filter Characteristics

The SVF topology provides 12 dB/octave (2-pole) rolloff in lowpass and highpass modes. The bandpass mode provides a 6 dB/octave slope on each side of the center frequency, with a peak gain that increases with resonance.

At zero resonance the filter has a smooth, gentle rolloff with no peak. As resonance approaches maximum, the filter produces a sharp, ringing peak at the cutoff frequency. The filter does not self-oscillate because the resonance is internally clamped to 0.99, keeping the damping factor slightly above zero.

### CV Modulation Behavior

The FREQ CV input is additive with the CUTOFF knob in the normalized 0.0-1.0 domain. Because the frequency mapping is exponential, this means:
- At low cutoff settings, a given CV voltage produces a smaller absolute frequency change but a similar perceived pitch change.
- At high cutoff settings, the same CV voltage produces a larger absolute frequency change.

A full +/-10V CV sweep covers the entire normalized range of +/-1.0, allowing external sources to sweep the filter across its full frequency range regardless of the knob position.

## Tips

- For classic subtractive synthesis, connect an oscillator to IN, set MODE to LP, and use an envelope module connected to the FREQ input to create filter sweeps. Set CUTOFF low (0.1-0.3) and RES moderately high (0.5-0.7) for pronounced "wah" effects as the envelope opens and closes the filter.
- Use HP mode to remove low-frequency rumble or DC offset from a signal. A CUTOFF value of around 0.05-0.1 (roughly 30-60 Hz) serves as a gentle high-pass cleanup filter.
- BP mode is effective for isolating a specific frequency band. Pair it with an LFO on the FREQ input to create a wah-wah or auto-filter effect that sweeps a narrow band across the spectrum.
- Increase RES to near-maximum values (0.8-0.95) for aggressive, resonant acid-style filter sounds. The filter will ring strongly at the cutoff frequency without self-oscillating, keeping things under control.
- Chain two GAFilter modules in series for steeper rolloff slopes. Two lowpass filters in series produce a 24 dB/octave (4-pole) response similar to a Moog-style ladder filter.
- Modulate the FREQ input with audio-rate signals from an oscillator for FM filter effects that create complex, inharmonic timbres.
- The coefficient dirty-checking means the filter is CPU-efficient when parameters are static. For patches with many filter instances, avoid unnecessary high-speed modulation of cutoff to keep CPU usage low.
- Use the same audio source split into two GAFilter modules set to LP and HP with matched cutoff frequencies to create a crossover, then process each band independently before mixing them back together.
