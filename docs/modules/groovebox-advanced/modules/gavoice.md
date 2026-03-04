# GAVoice

A complete synth voice module that combines a VCO (voltage-controlled oscillator), ADSR envelope, state-variable filter, and VCA into a single unit. GAVoice provides a self-contained pitched sound source that responds to gate and V/Oct signals, making it the fastest way to get a playable melodic voice inside a GrooveboxAdvanced patch.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| VOICE_GATE_IN | GATE | Trigger | Gate signal to trigger and hold the ADSR envelope. Signals above 1V are treated as high. Rising edges start the attack stage; falling edges start the release stage. |
| VOICE_VOCT_IN | V/O | Control (CV) | 1V/octave pitch control. 0V corresponds to C4 (261.63 Hz). The input is clamped to the range -10V to +10V. |
| VOICE_FM_IN | FM | Audio | Audio-rate frequency modulation input. The signal is clamped to +/-5V and scaled so that full-scale input produces roughly +/-50% frequency deviation. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| VOICE_OUT | OUT | Audio | Audio output in the standard VCV range of +/-5V. The output is the oscillator signal shaped by the filter and scaled by the ADSR envelope. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| WAVE | Dropdown | Sin, Saw, Sqr, Tri | Sin | Selects the oscillator waveform. Sine uses a 1024-entry lookup table with linear interpolation. Saw is a naive ramp from -1 to +1. Square is a hard-edged +1/-1 waveform with a fixed 50% duty cycle. Triangle is a piecewise linear waveform. |
| ATK | Knob | 0.001 - 2.0 s | 0.01 s | Attack time. How long the envelope takes to rise from zero to full level after a gate-on event. |
| DEC | Knob | 0.001 - 2.0 s | 0.1 s | Decay time. How long the envelope takes to fall from full level down to the sustain level. |
| SUS | Knob | 0.0 - 1.0 | 0.7 | Sustain level. The envelope level held while the gate remains high, after the attack and decay stages complete. |
| REL | Knob | 0.001 - 4.0 s | 0.3 s | Release time. How long the envelope takes to fall from the sustain level to zero after the gate goes low. |
| CUT | Knob | 0.0 - 1.0 | 0.5 | Filter cutoff frequency. Maps logarithmically from approximately 20 Hz (0.0) to 20 kHz (1.0). This is the base cutoff before envelope modulation is applied. |
| RES | Knob | 0.0 - 1.0 | 0.0 | Filter resonance. At 0.0 the filter has no resonant peak. At 1.0 the Q factor is at its narrowest (internally capped at 0.9 feedback to prevent instability), with self-oscillation tamed by a soft-clip (tanh) on the bandpass state. |
| E>F | Knob | 0.0 - 1.0 | 0.3 | Envelope-to-filter modulation depth. Controls how much the ADSR envelope opens the filter cutoff. At 0.0 the filter cutoff is static. At 1.0 the envelope adds up to the full normalized range on top of the base cutoff. The combined value is clamped to 0.01 - 0.99 before frequency conversion. |

## Details

### Signal Flow

The signal path inside GAVoice is: **VCO -> Filter -> VCA -> Output**.

1. **VCO**: The oscillator runs at a base frequency of C4 (261.63 Hz). The V/O input applies standard 1V/octave exponential pitch scaling via `baseFrequency * 2^(pitchCV)`. The FM input applies linear frequency modulation scaled to +/-10% of the current frequency per volt. The resulting frequency is clamped to the range 1 Hz - 20 kHz. The phase accumulator wraps in the 0-1 range using a fast floor-based modulo.

2. **ADSR Envelope**: A four-stage linear envelope triggered by the GATE input. The gate threshold is 1V (consistent with VCV Rack conventions). The envelope uses a linear ramp toward each stage's target at a rate of `1 / (stageTime * sampleRate)` per sample. Stage transitions use small epsilon values (0.001) for reliable detection. The envelope drives both the VCA and (optionally) the filter cutoff.

3. **Filter**: A state-variable filter (SVF) that simultaneously computes lowpass, highpass, and bandpass outputs. The module is hardcoded to use the lowpass output (filterMode is fixed at 0 internally). The cutoff frequency is computed as `20 * 1000^(cutoffMod)`, giving a logarithmic mapping from roughly 20 Hz to 20 kHz. The filter coefficient uses the standard `2 * sin(pi * freq / sampleRate)` formula, capped at 1.0 for stability. The bandpass state is soft-clipped with `tanh()` each sample to prevent resonance from blowing up.

4. **VCA**: The filtered signal is multiplied by the envelope level and a fixed gain of 5.0 to produce the standard VCV +/-5V audio output range. The final output is hard-clamped to +/-5V.

### Envelope-to-Filter Modulation

The E>F parameter adds the current envelope level (0.0 - 1.0) scaled by the E>F amount to the base cutoff value before converting to a frequency. This creates the classic "envelope opens the filter" effect commonly heard in subtractive synthesis. With a short attack and moderate decay, this produces percussive plucked tones. With longer attack times, it creates filter sweeps.

### Performance

The oscillator uses a 1024-entry sine lookup table with linear interpolation (via `GAFastMath::fastSin`) instead of `std::sin`, and the phase wrapping uses a fast floor-based modulo (`GAFastMath::fastFMod`) instead of `std::fmod`. These optimizations keep CPU usage low even with many voice instances.

## Tips

- Connect a StepTrig or PatGen module's gate output to the GATE input and a Sequencer or Quantize module to V/O for a basic melodic sequence.
- Use short ATK (0.001-0.005), moderate DEC (0.1-0.3), low SUS (0.0-0.2), and moderate E>F (0.4-0.7) with high CUT for classic acid-style plucked bass sounds.
- Set the waveform to Saw and increase RES to 0.5-0.7 for aggressive leads that cut through a mix.
- For pad-like sounds, use Tri or Sin waveforms with long ATK (0.5-1.0) and long REL (1.0-3.0) with low E>F.
- The FM input accepts audio-rate signals, so patching another oscillator (VCO or Carrier module) into FM creates metallic, bell-like, or harsh timbres depending on the modulator frequency and amplitude.
- Since the filter is always lowpass, use the CUT knob as a tone control: lower values darken the sound, higher values let the full harmonic content through.
- For thicker sounds, use two GAVoice modules with slightly different CUT and RES settings and mix them together with a Mixer module.
