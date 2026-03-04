# GASnareDrum

A synthesized snare drum module that generates a complete snare voice from three components: a sine oscillator for the drum body, filtered white noise for the snare wires, and a short noise burst for the attack transient. The balance between these three layers is independently adjustable, making it suitable for sounds ranging from tight, crisp electronic snares to longer, noisier percussive textures.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Trigger input for striking the snare drum. A rising edge crossing 1V resets the oscillator phase, fires all three envelopes (tone, noise, snap), and starts a new drum hit. Accepts standard VCV Rack trigger and gate signals. |
| 1 | VEL | Control (CV) | Velocity CV input. A unipolar 0-10V signal that scales the output amplitude. 5V produces unity gain, 10V produces 2x gain, and 0V silences the output. When this port is not connected, the snare plays at full level. |
| 2 | TONE | Control (CV) | Tone CV input. A bipolar signal that modulates the base oscillator frequency using exponential (pitch-like) scaling. The CV is clamped to the -5V to +5V range before application. Positive voltages raise the pitch, negative voltages lower it. The sensitivity is reduced (0.5 octaves per volt) relative to the standard 1V/octave convention, giving smoother tonal variation. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | OUT | Audio | Mono audio output. The combined tone oscillator, filtered noise, and snap transient signal, shaped by their respective envelopes and scaled to VCV Rack audio standard levels (approximately +/-5V at full amplitude). |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| TONE | Knob | 100.0 - 400.0 | 200.0 | Base frequency of the snare drum body oscillator in Hz. Lower values produce darker, deeper snare hits. Higher values produce brighter, more tonal snares. The effective frequency is further modified by the TONE CV input and an internal pitch-drop envelope that sweeps the oscillator downward from 1.5x the base frequency on each hit. |
| SNAP | Knob | 0.0 - 1.0 | 0.7 | Amplitude of the snap transient. This is a very short, unfiltered noise burst at the beginning of each hit that provides the sharp percussive attack. At 0.0, no snap is generated and the attack depends on the tone and noise envelopes alone. At 1.0, a prominent noise crack appears at the start of each hit. |
| NOISE | Knob | 0.0 - 1.0 | 0.6 | Amount of filtered noise mixed into the output, simulating snare wires. At 0.0, the snare has no wire noise and sounds like a pure tonal drum. At 1.0, the noise component is at full level, producing a bright, sizzling snare character. The noise is passed through a one-pole lowpass filter before mixing. |
| DECAY | Knob | 0.05 - 0.5 | 0.2 | Decay time in seconds controlling how quickly the snare fades out. This value governs all three envelopes (tone, noise, and snap), but each decays at a different rate. The tone envelope decays fastest (rate = 15/decay), the noise envelope decays slower (rate = 8/decay) to simulate the longer ring of snare wires, and the snap envelope has a fixed very fast decay regardless of this setting. |
| BODY | Knob | 0.0 - 1.0 | 0.6 | Amplitude of the sine oscillator body component. At 0.0, the tonal component is removed and the snare becomes a pure noise hit. At 1.0, the full sine oscillator body is mixed in, producing a more pitched, tonal snare sound. |

## Details

### Signal Flow

1. **Trigger Detection**: The module monitors the TRIG input for rising edges that cross a 1V threshold. When a trigger is detected, all three envelopes (tone, noise, snap) are reset to 1.0, and the oscillator phase is reset to zero for a consistent attack transient on every hit.

2. **Tone Envelope**: An exponential decay envelope shapes the sine oscillator body. The decay rate constant is `15.0 / decay`, where `decay` is the DECAY knob value. Because this rate is nearly twice that of the noise envelope, the tonal body of the snare fades faster than the wire noise, which is characteristic of real snare drum behavior where the shell resonance dies out before the snare wires stop rattling. Once the envelope falls below 0.0001, it is zeroed out to avoid denormalized floating-point numbers.

3. **Noise Envelope**: A second exponential decay envelope shapes the filtered noise component. It uses a decay rate of `8.0 / decay`, making it decay approximately 1.9 times slower than the tone envelope. This longer tail simulates the extended rattle of snare wires after the initial strike. Once below 0.0001, it is zeroed out.

4. **Snap Envelope**: A third exponential decay envelope shapes the snap transient. It uses a fixed decay rate of 200.0 (independent of the DECAY knob), producing a very short burst of approximately 5 milliseconds. This creates the initial "crack" or "pop" at the onset of the snare hit. Once below 0.0001, it is zeroed out.

5. **Frequency Calculation and Pitch Drop**: The oscillator frequency starts from the base TONE parameter value. If the TONE CV input has a non-negligible signal (above 0.001V), the CV is clamped to +/-5V and applied exponentially at a sensitivity of 0.5 octaves per volt using `exp2_taylor5(toneCV * 0.5)`. The resulting frequency is clamped to 50-500 Hz. On top of this, a pitch-drop effect is applied: the frequency is multiplied by `1.0 + toneEnvelope * 0.5`, so at the instant of trigger the oscillator starts at 1.5x the base frequency and sweeps down as the tone envelope decays. This downward sweep adds a characteristic percussive quality to the drum body.

6. **Sine Oscillator**: A phase-accumulating oscillator generates the drum body using a 1024-entry sine lookup table with linear interpolation. The phase advances by `currentTone * sampleTime` each sample and wraps to the 0-1 range. The oscillator output is multiplied by the tone envelope and then by the BODY parameter to set its level in the final mix.

7. **Filtered Noise Generator**: White noise is generated using `rand()` and scaled to the -1 to +1 range. This noise is passed through a one-pole lowpass filter with a coefficient of 0.7 (cutoff around 8 kHz at standard sample rates), which removes harsh high-frequency energy and gives the snare wires a more natural, slightly muffled character. The filtered noise is multiplied by the noise envelope and the NOISE parameter, then scaled by 0.8 before mixing.

8. **Snap Generator**: A separate white noise source generates the snap transient. Unlike the filtered noise, the snap noise is not filtered, giving it a full-bandwidth character with more high-frequency content for a sharp, cutting attack. The noise is multiplied by the snap envelope and the SNAP parameter, then scaled by 0.5 before mixing.

9. **Output Mixing**: The three components are summed as: `toneOutput * BODY + noiseOutput * 0.8 + snapOutput * 0.5`. If the VEL input is connected, the summed signal is further scaled by the velocity CV (with 5V = unity, as `velocity / 5.0`). The result is multiplied by 5.0 to produce standard VCV Rack audio levels.

### Envelope Timing Relationships

The three envelopes create a layered temporal structure in each snare hit:

- **Snap** (fixed rate 200.0): Decays in approximately 5ms regardless of DECAY setting. This is the initial transient crack.
- **Tone** (rate = 15.0 / decay): At DECAY = 0.2, the tone decays with a rate of 75, reaching near-zero in roughly 100ms. At DECAY = 0.5, the rate drops to 30 and the body rings for approximately 250ms.
- **Noise** (rate = 8.0 / decay): At DECAY = 0.2, the noise decays with a rate of 40, persisting for roughly 190ms. At DECAY = 0.5, the rate drops to 16 and the wire noise rings for approximately 470ms.

This hierarchy (snap first, then body, then wires last) mirrors the physical behavior of an acoustic snare drum.

### Pitch Drop Behavior

The pitch drop is not controlled by a separate parameter. It is always active and always sweeps by a factor of 1.5x at the moment of trigger. Because the pitch drop is tied to the tone envelope, its speed is governed by the DECAY parameter: shorter decays produce a faster, more aggressive pitch sweep, while longer decays produce a slower, more audible downward glide.

### Velocity Behavior

The VEL input uses a simple linear scaling model: the output is multiplied by `velocity / 5.0`. This means:
- 0V = silence (velocity zero)
- 5V = unity gain (full volume, same as if VEL were disconnected)
- 10V = 2x gain (louder than default, useful for accents)

Voltages below 0V are clamped to 0V, and voltages above 10V are clamped to 10V (maximum 2x gain).

### Output Level

The output signal is scaled to approximately +/-5V at peak amplitude (with no velocity modulation). Because the module does not apply any internal clipping or limiting, very high velocity values or extreme parameter combinations (high BODY + high NOISE + high SNAP simultaneously) could produce output levels exceeding +/-5V. The main GrooveboxAdvanced output limiter handles saturation at the final stage.

## Tips

- For a classic electronic snare (TR-808 style), set TONE to 180-220 Hz, DECAY to 0.15-0.25, BODY to 0.5-0.7, NOISE to 0.4-0.6, and SNAP to 0.5-0.7. This produces a punchy snare with a visible tonal body and moderate wire buzz.
- For a tight, short snare suited to fast tempos or drum-and-bass, reduce DECAY to 0.05-0.10, increase SNAP to 0.8-1.0, and lower BODY to 0.2-0.4. The short decay and strong snap create a crisp, defined hit that cuts through dense mixes.
- For a noise-only snare or hi-hat-like sound, set BODY to 0.0 and rely entirely on the NOISE and SNAP components. Shorter DECAY values with high NOISE produce a closed hi-hat character, while longer decays produce an open, airy texture.
- For a deeper, more tonal snare, lower TONE to 100-150 Hz and increase BODY to 0.8-1.0 while reducing NOISE. This produces a tom-like snare with a strong pitched component and minimal wire rattle.
- Use the TONE CV input with an LFO or random source to add subtle pitch variation across hits. Because the sensitivity is 0.5 octaves per volt, even moderate CV signals produce musical pitch changes without losing the snare character.
- Patch a velocity sequencer or step-based CV source into the VEL input to create accented snare patterns. Alternating between ghost notes (2-3V) and full hits (5V) adds groove and dynamic realism to programmed drum patterns.
- Follow GASnareDrum with a GAFilter in high-pass mode to thin out the body and emphasize the snare wire sizzle, or in low-pass mode to darken the snare for a more muffled, distant character.
- Layer GASnareDrum with GAKickDrum triggered simultaneously for a combined "kick-snare" layered hit, useful for breakbeat or industrial patterns. Adjust the relative velocity of each to set the balance.
- Route the output through GADistort at low-to-moderate drive settings to add saturation and grit, simulating the effect of an overdriven analog drum bus.
- For longer, more ambient snare tails, set DECAY to 0.4-0.5 and feed the output into GAReverb. The extended noise envelope provides natural material for the reverb to work with.
