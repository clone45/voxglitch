# GAVco

A voltage-controlled oscillator (VCO) that generates audio-rate waveforms tuned via the 1V/octave standard. It provides four classic waveform shapes, an FM modulation input for timbral variation, and a level control for output amplitude. The V/OCT input tracks pitch using an exponential frequency conversion, making it straightforward to play melodic content from sequencers or other CV sources within the GrooveboxAdvanced environment.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | V/O | Control (CV) | Volt-per-octave pitch input. Expects a standard 1V/oct signal clamped internally to the range -10V to +10V. At 0V the oscillator runs at its base frequency of 261.63 Hz (middle C / C4). Each +1V doubles the frequency (one octave up) and each -1V halves it (one octave down). The conversion uses a fast Taylor-series approximation of 2^v for efficiency. |
| 1 | FM | Control (CV) | Frequency modulation input. Accepts a signal clamped internally to the range -10V to +10V. The incoming value is applied as a linear FM index using the formula `frequency * (1 + fmInput)`, where `fmInput` is the raw voltage. At 0V the pitch is unaffected. Positive values increase the instantaneous frequency; negative values decrease it. At -1V the modulated frequency reaches zero, and values below -1V would invert the phase direction, though the final frequency is hard-clamped to the range 0.01 Hz to 20,000 Hz. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | The oscillator's audio output, scaled to VCV Rack's audio standard of +/-5V at full level. The actual peak voltage is `5.0 * level`, where level is set by the LEVEL knob. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| WAVE | Dropdown | Sin, Saw, Sqr, Tri | Sin | Selects the oscillator waveform shape. Sin produces a sine wave via a 1024-entry lookup table with linear interpolation. Saw produces an upward ramp from -1 to +1 (computed as `2 * phase - 1`). Sqr produces a square wave that outputs +1 for the first half of the cycle and -1 for the second half, with no anti-aliasing. Tri produces a triangle wave using a three-segment piecewise linear function that rises from 0 to +1 in the first quarter, falls from +1 to -1 over the middle half, and rises from -1 to 0 in the final quarter. |
| LEVEL | Knob | 0.0 - 1.0 | 1.0 | Output amplitude multiplier. Scales the generated waveform before the final 5V output scaling is applied. At 0.0 the output is silent. At 1.0 (default) the output swings the full +/-5V range. Intermediate values attenuate proportionally, so 0.5 produces a +/-2.5V signal. |

## Details

### Signal Flow

1. **Pitch Input**: The V/OCT input voltage is read and clamped to the range -10V to +10V. The base frequency of 261.63 Hz (C4) is multiplied by `2^(vOct)` using VCV Rack's `dsp::exp2_taylor5` fast approximation. This converts the standard 1V/octave signal into an absolute frequency in hertz.

2. **FM Modulation**: The FM input voltage is read and clamped to -10V to +10V. It is applied as through-zero linear FM using the formula `frequency * (1.0 + fmInput)`. This means the modulation depth is proportional to the carrier frequency -- at higher pitches, the same FM voltage produces wider deviations in hertz, maintaining a consistent timbral relationship across the keyboard. The resulting frequency is hard-clamped to the range 0.01 Hz to 20,000 Hz to prevent runaway values or negative frequencies.

3. **Phase Accumulation**: The internal phase accumulator advances by `modulatedFreq * sampleTime` each sample, where sampleTime is the reciprocal of the engine sample rate. The phase is wrapped to the [0, 1) range using a fast floor-based modulo function that includes a guard against NaN and infinity values that could arise from extreme FM modulation.

4. **Waveform Generation**: The current phase value (0 to 1, representing one full cycle) is converted to a raw output sample in the range -1 to +1 based on the selected waveform:
   - **Sine**: Uses a 1024-entry precomputed sine lookup table with linear interpolation between adjacent entries. The table covers one full cycle, and index wrapping uses a bitmask for speed.
   - **Saw**: Computed as `2.0 * phase - 1.0`, producing a rising ramp that starts at -1 when phase is 0 and reaches +1 just before phase wraps. This is a naive (non-band-limited) sawtooth.
   - **Square**: A hard comparison -- outputs +1.0 when phase is below 0.5 and -1.0 when phase is at or above 0.5. This is a naive square wave with no anti-aliasing, so some aliasing artifacts may be audible at higher frequencies.
   - **Triangle**: A three-piece linear function. Phase 0 to 0.25 ramps from 0 to +1. Phase 0.25 to 0.75 ramps from +1 down to -1. Phase 0.75 to 1.0 ramps from -1 back up to 0.

5. **Level and Output Scaling**: The raw waveform sample (-1 to +1) is multiplied by the LEVEL parameter, then multiplied by 5.0 to produce the final output in VCV Rack's standard audio voltage range. At full level, peak-to-peak swing is 10V (+/-5V).

### CV Modulation Behavior

The V/OCT input uses standard exponential (1V/octave) pitch tracking. Equal voltage increments produce equal musical intervals. For example, a C major scale played from a sequencer outputting 0V, 0.167V, 0.333V, 0.417V, 0.583V, 0.75V, 0.917V, 1.0V will produce the expected pitch intervals.

The FM input uses linear frequency modulation. Unlike exponential FM, linear FM preserves harmonic relationships regardless of the modulation depth, making it well-suited for FM synthesis timbres. Because the modulation formula is `freq * (1 + fmInput)`, an FM signal of +1V doubles the instantaneous frequency and -1V brings it to zero. Signals beyond -1V can create through-zero FM effects, though the frequency clamp at 0.01 Hz prevents true negative frequencies.

### Performance Notes

The module uses several optimizations for real-time DSP efficiency. The sine waveform is generated from a precomputed 1024-entry lookup table rather than calling `std::sin` on every sample. Phase wrapping uses a `floor()`-based technique that benchmarks at roughly 3-6x faster than `std::fmod`. The V/OCT conversion uses a 5th-order Taylor series approximation of `2^x` rather than the standard library `pow` function.

## Tips

- Connect a sequencer's CV output to the V/O input to play melodic patterns. The 1V/oct tracking means standard pitch CVs from any GrooveboxAdvanced sequencer module (Sequencer, Sequencer16, PatArp) will produce correctly tuned notes.
- For FM synthesis timbres, patch another VCO's output into the FM input. Start with a sine wave on both oscillators and gradually increase the modulator's level to add harmonic complexity. Simple integer frequency ratios between carrier and modulator (2:1, 3:2, etc.) produce harmonic timbres, while non-integer ratios create inharmonic, bell-like tones.
- Use the LEVEL knob to mix the VCO down before sending it to a mixer or output. This is especially useful when layering multiple oscillators -- keep individual levels moderate to avoid clipping at the mixer stage.
- Patch an LFO into the FM input at low depth for vibrato. Because the FM input applies linear modulation, the vibrato depth in hertz will scale with pitch, which can sound more natural than fixed-depth vibrato at some settings.
- For thicker sounds, use two VCO modules with slightly different V/OCT offsets (detune) and mix their outputs. Even a fraction of a volt offset on one VCO's pitch creates a chorusing effect from the beating frequencies.
- The square and saw waveforms are naive (non-band-limited), so they will produce some aliasing at higher frequencies. For cleaner timbres in the upper register, prefer the sine waveform or follow the VCO with a low-pass filter to tame aliasing artifacts.
- Route an envelope into the FM input for percussive FM "pluck" sounds. A short, decaying envelope creates a burst of harmonic content at the note onset that quickly settles to the pure carrier tone, mimicking the attack transient of plucked or struck instruments.
