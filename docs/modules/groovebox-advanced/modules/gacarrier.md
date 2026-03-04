# GACarrier

A carrier oscillator module that generates audio-rate waveforms. In FM synthesis terminology, the carrier is the oscillator whose output you actually hear, as opposed to modulators which shape the carrier's timbre. GACarrier supports four standard waveform shapes and accepts both frequency CV and FM modulation inputs.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | FM | Audio | Frequency modulation input. Scales the oscillator frequency by (1 + FM), so a value of 1.0 doubles the frequency and -0.5 halves it. Clamped to +/-10V before processing. |
| 1 | FREQ | Control (CV) | Exponential frequency CV input following the 1 volt/octave standard. A CV of +1.0 doubles the base frequency; -1.0 halves it. Clamped to +/-10V. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | Audio output scaled to VCV Rack standard levels (+/-5V). The raw oscillator waveform (range -1 to +1) is multiplied by the LEVEL parameter and then scaled by 5.0. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| FREQ | Knob | 8.0 - 20000.0 Hz | 440.0 | Base oscillator frequency in Hertz. This value is further modified by the FREQ CV and FM inputs before waveform generation. |
| WAVE | Dropdown | Sin / Saw / Sqr / Tri | Sin | Selects the oscillator waveform shape. Sine uses a 1024-entry lookup table with linear interpolation. Saw is a naive ramp from -1 to +1. Square outputs +1 for the first half of the cycle and -1 for the second half. Triangle is a piecewise linear waveform that ramps between -1 and +1. |
| LEVEL | Knob | 0.0 - 1.0 | 1.0 | Output amplitude scaling applied before the final 5V normalization. At 0.0 the output is silent; at 1.0 the output reaches the full +/-5V range. |

## Details

### Signal Flow

The DSP processing chain operates in this order:

1. **Base frequency**: The FREQ knob value (8--20000 Hz) is used as the starting frequency.
2. **Exponential frequency CV**: The FREQ input applies exponential (1V/oct) scaling using `exp2_taylor5`. A +1V signal doubles the frequency; +2V quadruples it, and so on. The CV is clamped to +/-10V to prevent overflow.
3. **FM modulation**: The FM input applies linear frequency modulation by multiplying the current frequency by `(1 + fmInput)`. The FM signal is clamped to +/-10V before application. This means an FM input of 0 has no effect, positive values increase frequency, and negative values decrease it.
4. **Frequency clamping**: After both CV and FM are applied, the final frequency is clamped to the range 0.01--20000 Hz to prevent runaway oscillation or negative frequencies.
5. **Phase accumulation**: The phase advances by `frequency * sampleTime` each sample and wraps to the [0, 1) range using a fast floor-based modulo operation. A `NaN`/`Inf` safety check is included.
6. **Waveform generation**: The phase is converted to an output sample according to the selected waveform shape.
7. **Output scaling**: The waveform sample is multiplied by the LEVEL parameter, then scaled by 5.0 to produce VCV Rack standard audio levels (+/-5V at full level).

### Waveform Details

- **Sine**: Uses a 1024-point lookup table with linear interpolation for fast, CPU-efficient sine generation. This is not band-limited, but the lookup table provides clean output for most audio frequencies.
- **Saw**: A simple `2 * phase - 1` ramp. Not band-limited, so aliasing will be audible at higher frequencies.
- **Square**: Hard threshold at the 50% duty cycle point. Not band-limited.
- **Triangle**: Piecewise linear with four segments per cycle. Not band-limited, but the triangle shape has naturally weak harmonics so aliasing is less pronounced.

### Phase Reset

The oscillator phase resets to 0.0 when `reset()` is called, which occurs at the start of each new sequencer step.

## Tips

- Use GACarrier as the primary sound source in an FM patch by connecting a GAModulator or GAOperator to the FM input. Deeper FM input values produce more complex, harmonically rich timbres.
- Connect an GAEnvelope to the LEVEL parameter (via a VCA) to shape the amplitude of the carrier over time for standard subtractive-style patches.
- The FREQ CV input follows 1V/oct, so you can drive it from a sequencer or quantizer for pitched melodies. Combine this with a GAModulator on the FM input for evolving FM bass or lead sounds.
- For detuned unison effects, use two GACarrier modules with slightly different FREQ values and mix them together using a GAMixer.
- The saw and square waveforms are naive (non-band-limited), which can produce aliasing artifacts at high frequencies. This can be a desirable lo-fi character or you can follow the carrier with a GAFilter in low-pass mode to tame the highs.
- Setting the FM input to audio-rate signals from another oscillator creates classic FM synthesis timbres. Start with simple integer frequency ratios (2:1, 3:2) between the modulator and carrier for harmonic results.
