# GALFO

A low-frequency oscillator (LFO) that generates periodic control voltages for modulating other modules. It offers five waveform shapes, switchable frequency range, and bipolar or unipolar output modes. The RATE input allows external CV to exponentially scale the oscillation frequency, and the RST input resets the phase on demand for tight synchronization with rhythmic events.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | RATE | Control (CV) | Rate modulation CV input. Expects a signal in the standard +/-5V range. The incoming voltage is normalized to +/-1 internally and then applied as an exponential frequency multiplier: +5V multiplies the base rate by 32x, -5V divides it by 32x, and 0V leaves the rate unchanged. This gives approximately 10 octaves of CV-controlled frequency range. |
| 1 | RST | Trigger | Phase reset trigger input. A rising edge crossing the 0.5V threshold resets the LFO phase to zero, causing the waveform to restart from the beginning of its cycle. Subsequent samples below 0.5V re-arm the trigger detector. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Control (CV) | The LFO waveform output. In bipolar mode the signal swings from -5V to +5V (VCV Rack bipolar standard). In unipolar mode the signal ranges from 0V to 10V (VCV Rack unipolar standard). |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| RATE | Knob (exponential) | 0.0001 - 8.0 Hz | 0.5 Hz | Base oscillation frequency in hertz. The knob uses an exponential taper, giving fine control at low frequencies and broad sweeps at the high end. In Low range mode the effective base rate spans 0.0001 Hz to 8.0 Hz. In High range mode this base value is multiplied by 20, spanning 0.002 Hz to 160 Hz before any CV modulation is applied. |
| WAVE | Dropdown | Sin, Saw, Sqr, Tri, S&H | Sin | Selects the output waveform shape. Sin produces a smooth sine wave via a 1024-point lookup table with linear interpolation. Saw produces a downward ramp from +1 to -1. Sqr produces a square wave that outputs +1 for the first half of the cycle and -1 for the second half. Tri produces a triangle wave that ramps linearly up and then down. S&H (Sample and Hold) latches a new random value each time the phase wraps around, holding that value constant until the next cycle completes. |
| RANGE | Switch | Low / High | Low | Frequency range selector. In Low mode the LFO operates at the base RATE knob frequency (0.0001 - 8.0 Hz before CV), suitable for typical modulation duties. In High mode the base rate is multiplied by 20x, pushing the frequency range up to audio-rate territory (clamped to a maximum of 2000 Hz after CV modulation). The overall clamped ranges are: Low mode 0.0001 - 50 Hz, High mode 0.02 - 2000 Hz. |
| OUTPUT | Switch | Uni / Bi | Bi | Output polarity mode. In Bi (bipolar) mode the waveform is scaled to +/-5V, centered around 0V. In Uni (unipolar) mode the waveform is offset and scaled to 0-10V, with the bottom of the waveform at 0V and the peak at 10V. |

## Details

### Signal Flow

1. **Input Processing**: On each sample, the process method reads the RATE CV input and the RST trigger input. The RATE CV is divided by 5.0 to normalize a +/-5V signal into a +/-1 internal range, then clamped to that range.

2. **Reset Detection**: The RST input is checked for a rising edge by comparing the current sample against a 0.5V threshold while tracking the previous sample's value. When a rising edge is detected, the internal phase accumulator is set to zero, restarting the waveform from the beginning of its cycle.

3. **Rate Calculation**: The base rate from the RATE knob is optionally multiplied by 20 if the RANGE switch is set to High. The normalized rate CV is then converted to an exponential multiplier using the formula `2^(rateCV * 5)`, which yields a range of 1/32x to 32x. This multiplier is applied to the base rate. The final modulated rate is clamped to prevent extreme values: 0.0001 - 50 Hz in Low mode, 0.02 - 2000 Hz in High mode.

4. **Phase Accumulation**: The phase accumulator advances by `modulatedRate * sampleTime` each sample, where sampleTime is the reciprocal of the engine sample rate. The phase is wrapped to the 0-1 range using a fast floor-based modulo operation that also guards against NaN and infinity values from extreme modulation.

5. **Waveform Generation**: The current phase value (0 to 1, representing one full cycle) is converted to an output sample based on the selected waveform:
   - **Sine**: Uses a 1024-entry lookup table with linear interpolation between adjacent entries. This avoids calling `std::sin` on every sample while maintaining good accuracy.
   - **Saw**: Computed as `1.0 - phase * 2.0`, producing a downward ramp from +1 at phase 0 to -1 at phase 1.
   - **Square**: Outputs +1.0 when the phase is in the first half of the cycle (0 to 0.5) and -1.0 in the second half (0.5 to 1.0). This is a hard-edged waveform with no anti-aliasing.
   - **Triangle**: A piecewise linear waveform that rises from 0 to +1 during the first quarter, falls from +1 to -1 during the middle half, and rises from -1 to 0 during the last quarter.
   - **Sample and Hold**: Detects phase wraparound (where the current phase is less than the previous phase) and generates a new random value between -1 and +1 at each wrap. The value is held constant between wraps.

6. **Output Scaling**: The internally generated waveform ranges from -1 to +1. If the OUTPUT switch is set to Bi (bipolar), the output is multiplied by 5.0 to produce a +/-5V signal. If set to Uni (unipolar), the waveform is first shifted to the 0-1 range via `(output + 1) * 0.5`, then multiplied by 10.0 to produce a 0-10V signal.

### CV Modulation Behavior

The rate CV applies exponential (V/Oct-style) scaling rather than linear scaling. This means that equal voltage changes produce equal pitch-ratio changes. For example, a +1V change at the RATE input always doubles the oscillation frequency regardless of the current base rate. The 5-octave-per-volt sensitivity means the full +/-5V input range covers roughly 10 octaves of frequency modulation. Because the CV is clamped to +/-1 (after normalization), signals exceeding +/-5V do not produce additional modulation.

## Tips

- For standard vibrato, select the Sine waveform, set RANGE to Low, OUTPUT to Bi, and patch the OUT into a voice module's frequency modulation input. A rate around 4-6 Hz with subtle attenuation produces a natural vibrato effect.
- Use the Saw waveform in unipolar mode to create rising ramp envelopes that repeat at the LFO rate. This is useful for driving filter sweeps that reset periodically, producing rhythmic "wah" effects.
- Patch a clock or gate signal into the RST input to synchronize the LFO phase with your sequencer. This ensures the modulation waveform always starts from the same point relative to each beat, keeping rhythmic modulation patterns tight and predictable.
- The S&H waveform generates a new random voltage each cycle, creating stepped random modulation. Patch this into a filter cutoff or pitch input for generative, evolving textures. Increase the RATE for rapid random changes or decrease it for slow, drifting randomness.
- Switch to High range mode to push the LFO into audio-rate territory. Patching an audio-rate LFO into a voice module's frequency input produces FM synthesis effects. The Sine waveform in this mode creates the cleanest FM timbres, while Square and Saw produce harsher, more complex spectra.
- Use the RATE CV input to create an LFO whose speed is controlled by another LFO or an envelope. For example, patching an envelope into RATE produces modulation that speeds up during the attack phase and slows during release, useful for dramatic sweeps and risers.
- When using unipolar mode (0-10V) for amplitude modulation through a VCA, the signal covers the full positive range without needing an offset. When using bipolar mode (+/-5V) for pitch modulation, the oscillation is centered around zero, which keeps the average pitch unaffected.
