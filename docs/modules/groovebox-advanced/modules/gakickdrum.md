# GAKickDrum

A synthesized kick drum module inspired by the Nord Modular G1 architecture. It generates a complete kick drum voice from a sine oscillator with a pitch envelope sweep, a noise-based click transient, and an exponential amplitude decay. Suitable for a wide range of kick sounds from deep sub-bass thuds to punchy electronic kicks with sharp transient attacks.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Trigger input for striking the kick drum. A rising edge crossing 1V resets the oscillator phase, fires the amplitude and pitch envelopes, and generates a new click transient. Accepts standard VCV Rack trigger and gate signals. |
| 1 | VEL | Control (CV) | Velocity CV input. A unipolar 0-10V signal that scales the output amplitude. 5V produces unity gain, 10V produces 2x gain, and 0V silences the output. When this port is not connected, the kick plays at full level. |
| 2 | PTCH | Control (CV) | Pitch CV input. A bipolar signal following the 1V/octave standard that transposes the base pitch up or down. The CV is clamped to the -5V to +5V range before application, providing up to 5 octaves of transposition in either direction. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | OUT | Audio | Mono audio output. The combined sine oscillator and click transient signal, shaped by the amplitude envelope and scaled to VCV Rack audio standard levels (approximately +/-5V at full amplitude). |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| PITCH | Knob | 20.0 - 200.0 | 60.0 | Base frequency of the kick drum oscillator in Hz. Lower values produce deep, sub-bass kicks. Higher values produce tighter, more tonal kicks. The final frequency is further modified by the PTCH CV input and the pitch envelope. |
| CLICK | Knob | 0.0 - 1.0 | 0.5 | Amplitude of the noise-burst click transient. At 0.0, no click is generated and the attack relies solely on the pitch envelope sweep. At 1.0, a prominent noise click is mixed with the oscillator at the start of each hit, adding a sharp percussive attack. |
| DECAY | Knob | 0.05 - 1.0 | 0.3 | Decay time of the amplitude envelope. Controls how quickly the kick drum fades out after being triggered. Lower values produce short, tight kicks. Higher values produce longer, booming kicks with extended sustain. The pitch envelope decay is derived from this value but decays approximately 10 times faster. |
| PUNCH | Knob | 0.0 - 1.0 | 0.5 | Depth of the pitch envelope sweep. At 0.0, the oscillator stays at the base pitch throughout the hit. At 1.0, the oscillator starts at up to 4 times the base frequency (2 octaves above) and sweeps down to the base pitch as the pitch envelope decays. This downward pitch sweep is the characteristic sound of a synthesized kick drum. |

## Details

### Signal Flow

1. **Trigger Detection**: The module monitors the TRIG input for rising edges that cross a 1V threshold. When a trigger is detected, the oscillator phase is reset to zero, both the amplitude and pitch envelopes are set to 1.0, and a random noise sample is generated for the click transient. The click counter is set to a duration of 3 milliseconds.

2. **Amplitude Envelope**: An exponential decay envelope shapes the overall volume of the kick. The decay rate constant is calculated as `7.0 / decay`, where `decay` is the DECAY knob value. This means the envelope reaches approximately 0.1% of its initial value after the time specified by the DECAY parameter. Once the envelope falls below 0.0001, it is zeroed out to avoid denormalized floating-point numbers.

3. **Pitch Envelope**: A second exponential decay envelope controls the pitch sweep. It uses a decay rate of `70.0 / decay`, making it decay 10 times faster than the amplitude envelope. This fast decay creates the characteristic downward pitch sweep at the start of the kick that quickly settles to the base frequency.

4. **Frequency Calculation**: The current oscillator frequency is computed by starting from the base PITCH value, applying the PTCH CV input using 1V/octave scaling (via `exp2_taylor5`), and then adding the pitch envelope contribution. The pitch envelope multiplier is `1 + pitchEnvelope * punch * 3`, so at the moment of trigger with PUNCH at 1.0, the oscillator starts at 4 times the base frequency and sweeps down. The resulting frequency is clamped to the 10-500 Hz range.

5. **Sine Oscillator**: A phase-accumulating sine oscillator generates the drum body. The phase advances by `currentFreq * sampleTime` each sample, and a fast sine approximation produces the output waveform. Because the phase is reset on each trigger, the attack waveform is consistent regardless of when the trigger arrives.

6. **Click Generator**: When triggered, a 3-millisecond noise burst is generated. A new random sample is produced every audio sample (creating white noise), and its amplitude is shaped by a linear fade-out envelope over the 3ms duration. The noise level is scaled by the CLICK parameter. This click mixes additively with the sine oscillator before the amplitude envelope is applied.

7. **Output Mixing**: The sine oscillator output and click transient are summed, then multiplied by the amplitude envelope. If the VEL input is connected, the signal is further scaled by the velocity CV (with 5V = unity). The result is multiplied by 5.0 to produce standard VCV Rack audio levels.

### Pitch Envelope Behavior

The punch parameter controls the depth of the pitch sweep, not its speed. The speed of the pitch sweep is always tied to the DECAY parameter (but 10x faster). This means:
- Short DECAY + high PUNCH = a fast, aggressive pitch "zap" that quickly settles.
- Long DECAY + high PUNCH = a slower, more audible pitch sweep, producing a more tonal "boing" character.
- Any DECAY + zero PUNCH = no pitch sweep at all; the oscillator stays at the base frequency.

### Velocity Behavior

The VEL input uses a simple linear scaling model: the output is multiplied by `velocity / 5.0`. This means:
- 0V = silence (velocity zero)
- 5V = unity gain (full volume, same as if VEL were disconnected)
- 10V = 2x gain (louder than default, useful for accents)

Voltages above 10V will produce gains above 2x, which may cause clipping in downstream modules.

### Output Level

The output signal is scaled to approximately +/-5V at peak amplitude (with no velocity modulation). Because the module does not apply any internal clipping or limiting, very high velocity values or extreme parameter settings could produce output levels exceeding +/-5V. Use a downstream limiter or attenuator if strict level control is required.

## Tips

- For a classic TR-808-style kick, set PITCH to around 50-60 Hz, DECAY to 0.4-0.6, PUNCH to 0.5-0.7, and CLICK to 0.1-0.2. This produces a deep, boomy kick with a subtle pitch sweep and minimal click.
- For a tight, punchy electronic kick suitable for techno or house, use PITCH around 50 Hz, DECAY at 0.1-0.2, PUNCH at 0.6-0.8, and CLICK at 0.3-0.5. The short decay and strong punch create a snappy, defined hit.
- For a sub-bass kick with no tonal sweep, set PUNCH to 0.0 and PITCH to 30-40 Hz with a moderate DECAY. This produces a pure low-frequency thump useful for layering underneath a more textured kick.
- Use the PTCH CV input with a sequencer or random source to create melodic bass drum patterns where the pitch changes per step. Because the pitch input follows 1V/octave, quantized pitch sequences will produce musically related kick tones.
- Patch a velocity sequencer (such as GAStepNum) into the VEL input to create accented kick patterns. Alternating between lower and higher velocity values adds groove and dynamics to repetitive kick patterns.
- For lo-fi or industrial kick sounds, increase the CLICK parameter to 0.8-1.0. The noise burst becomes a prominent part of the sound, adding a gritty, distorted character to the attack.
- Follow GAKickDrum with a GAFilter set to low-pass mode to remove high-frequency click harmonics and shape the kick's tonal character. This is especially useful when using high CLICK or PUNCH settings that introduce upper harmonics.
- Chain GAKickDrum's output through GADistort at moderate drive settings to add saturation and harmonic density, simulating the effect of overdriving an analog drum machine output.
