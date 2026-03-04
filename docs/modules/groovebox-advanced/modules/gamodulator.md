# GAModulator

A modulator oscillator module designed for FM (frequency modulation) synthesis. Unlike a carrier, the modulator's output is not typically listened to directly -- instead it is patched into the FM input of a carrier, operator, or another modulator to shape the timbre of the audible signal. GAModulator supports four waveform shapes and provides both FM cascading and exponential frequency CV inputs, making it suitable for building multi-operator FM synthesis chains.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | FM | Control (CV) | Cascaded FM modulation input. Scales the oscillator frequency by (1 + FM), so a value of 1.0 doubles the frequency and -0.5 halves it. Clamped to +/-10V before processing. Allows chaining multiple modulators together for complex FM topologies. |
| 1 | FREQ | Control (CV) | Exponential frequency CV input following the 1 volt/octave standard. A CV of +1.0 doubles the base frequency; -1.0 halves it. Clamped to +/-10V. Uses `exp2_taylor5` for exponential conversion. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Control (CV) | Modulation output scaled to VCV Rack standard levels (+/-5V). The raw oscillator waveform (range -1 to +1) is multiplied by the DEPTH parameter and then scaled by 5.0. Intended to drive the FM input of a carrier, operator, or another modulator. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| RATIO | Knob | 0.25 - 32.0 | 2.0 | Frequency ratio relative to the internal base frequency (440 Hz). The oscillator runs at `baseFrequency * RATIO`, so a RATIO of 2.0 produces 880 Hz. Integer ratios (1, 2, 3, ...) yield harmonically related timbres when modulating a carrier at the same base frequency; non-integer ratios produce inharmonic, bell-like or metallic textures. |
| WAVE | Dropdown | Sin / Saw / Sqr / Tri | Sin | Selects the modulator waveform shape. Sine is the classic FM modulation waveform and produces the most predictable harmonic sidebands. Saw, square, and triangle introduce additional harmonic content into the modulation signal, creating denser, more complex spectra on the carrier. |
| DEPTH | Knob | 0.0 - 4.0 | 1.0 | Modulation depth (output level). Multiplies the raw waveform before the final 5V scaling. At 0.0 the output is silent, producing no modulation. Values above 1.0 overdrive the modulation for aggressive FM timbres. The maximum output voltage is +/-20V (at DEPTH = 4.0). |

## Details

### Signal Flow

The DSP processing chain operates in this order:

1. **Base frequency with ratio**: The oscillator frequency starts at `baseFrequency * RATIO`, where `baseFrequency` is an internal reference of 440 Hz. A RATIO of 2.0 yields 880 Hz.
2. **Exponential frequency CV**: The FREQ input applies exponential (1V/oct) scaling using `exp2_taylor5`. A +1V signal doubles the frequency; +2V quadruples it. The CV is clamped to +/-10V to prevent overflow in the exponential function.
3. **Cascaded FM modulation**: The FM input applies linear frequency modulation by multiplying the current frequency by `(1 + fmInput)`. The FM signal is clamped to +/-10V before application. An FM input of 0 has no effect, positive values increase frequency, and negative values decrease it.
4. **Frequency clamping**: After both CV and FM are applied, the final frequency is clamped to the range 0.01--20000 Hz to prevent runaway oscillation or negative frequencies.
5. **Phase accumulation**: The phase advances by `frequency * sampleTime` each sample and wraps to the [0, 1) range using a fast floor-based modulo operation. A `NaN`/`Inf` safety check is included in the wrapping.
6. **Waveform generation**: The phase is converted to an output sample according to the selected waveform.
7. **Output scaling**: The waveform sample is multiplied by the DEPTH parameter, then scaled by 5.0 to produce the final modulation output.

### Waveform Details

- **Sine**: Uses a 1024-point lookup table with linear interpolation for fast, CPU-efficient sine generation. This is the classic FM modulation waveform and produces clean, predictable sideband patterns on the carrier.
- **Saw**: A simple `2 * phase - 1` ramp. Not band-limited. Produces asymmetric modulation that adds a wider spread of harmonics to the carrier.
- **Square**: Hard threshold at the 50% duty cycle point, outputting +1 or -1. Produces abrupt frequency jumps on the carrier, which creates harsh, buzzy timbres.
- **Triangle**: Piecewise linear with four segments per cycle. Produces a softer modulation effect than saw or square, with fewer high-order sidebands than sine.

### Phase Reset

The oscillator phase resets to 0.0 when `reset()` is called, which occurs at the start of each new sequencer step. This ensures consistent timbral attacks across steps.

### FM vs. Carrier Distinction

GAModulator is categorized as an oscillator (GA_CAT_OSCILLATORS), but its ports are typed as `GA_PORT_MODULATION` rather than `GA_PORT_AUDIO`, reflecting its intended role as a modulation source. Its output should be patched into the FM input of other oscillator modules rather than directly to an output module.

## Tips

- For classic FM synthesis, connect a GAModulator's OUT to the FM input of a GACarrier. Use integer RATIO values (1, 2, 3, 4) relative to the carrier frequency for harmonic timbres. A modulator-to-carrier ratio of 1:1 produces a "buzzy" timbre; 2:1 produces octave-related harmonics; 3:1 and higher create increasingly bright metallic tones.
- Chain multiple GAModulators together by connecting one modulator's OUT to another's FM input, then into a carrier. This creates cascaded FM (a stack of modulators), which produces extremely rich and complex spectra. Start with low DEPTH values on the inner modulators to keep the sound manageable.
- Increase DEPTH beyond 1.0 for aggressive, distorted FM tones. DEPTH acts as the modulation index -- higher values create more sidebands and a wider spectrum. Automate DEPTH with a GAEnvelope via a VCA for timbres that evolve over time (bright attack that mellows into a sine-like sustain).
- Non-integer RATIO values (e.g., 1.414, 2.76) produce inharmonic spectra suitable for bells, gongs, and metallic percussion. Combine with a GAEnvelope on the carrier for realistic percussive FM patches.
- Use the FREQ CV input to track pitch alongside the carrier when building FM patches that need to play melodies. Sending the same V/oct signal to both the carrier's FREQ CV and the modulator's FREQ CV keeps the modulation ratio consistent across the keyboard.
- Try the saw or square waveforms for the modulator to break out of traditional sine-based FM. These waveforms inject additional harmonic content into the modulation signal itself, creating dense, noisy textures that work well for industrial or experimental sound design.
