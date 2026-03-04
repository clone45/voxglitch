# GAEnvelope

An ADSR envelope generator that produces a shaped control voltage in response to gate signals. When a gate goes high, the envelope rises through its attack phase, falls through decay to the sustain level, holds there for the duration of the gate, and then falls to zero through the release phase when the gate goes low. The output is a unipolar 0-10V control signal suitable for driving VCAs, filter cutoffs, or any other modulation destination.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | GATE | Gate | Gate input that triggers the envelope. A rising edge (crossing above 0.5V) starts the attack phase. A falling edge (crossing below 0.5V) starts the release phase. The envelope sustains for as long as the gate remains high. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 1 | OUT | Control (CV) | The envelope output, scaled from 0V to 10V (VCV Rack unipolar standard). At the peak of the attack phase the output reaches 10V. During sustain it holds at the sustain level multiplied by 10V. After release completes, the output rests at 0V. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| ATK | Knob | 0.001 - 2.0 | 0.01 | Attack time in seconds. Controls how long the envelope takes to rise from 0V to 10V after a gate-on event. At the minimum value (1 ms) the attack is nearly instantaneous; at the maximum (2 s) it produces a slow fade-in. |
| DEC | Knob | 0.001 - 2.0 | 0.1 | Decay time in seconds. Controls how long the envelope takes to fall from the peak (10V) down to the sustain level after the attack phase completes. At the minimum (1 ms) the transition is abrupt; at the maximum (2 s) it is a gradual descent. |
| SUS | Knob | 0.0 - 1.0 | 0.7 | Sustain level as a fraction of full scale. At 1.0 the envelope holds at 10V after decay (effectively making the decay phase inaudible). At 0.0 the envelope decays all the way to silence and the sustain phase outputs 0V. At the default of 0.7, the sustain holds at 7V. |
| REL | Knob | 0.001 - 4.0 | 0.3 | Release time in seconds. Controls how long the envelope takes to fall from the current level to 0V after a gate-off event. At the minimum (1 ms) the release is a sharp cutoff; at the maximum (4 s) it produces a long, smooth tail. |

## Details

### Signal Flow

1. **Gate Edge Detection**: On each sample, the process method reads the GATE input and compares it against a 0.5V threshold. The module tracks the previous gate state internally. Only transitions (low-to-high or high-to-low) trigger state changes in the ADSR engine -- holding the gate high or low does not repeatedly retrigger.

2. **ADSR Engine**: The envelope uses Nigel Redmon's ADSR implementation from EarLevel Engineering. All stages use exponential curves computed via pre-calculated coefficients, giving the envelope a natural, analog-like shape rather than linear ramps. The attack curve is concave (fast initial rise that slows near the peak), while the decay and release curves are convex (fast initial drop that slows toward the target level). These curve shapes are determined by internal target ratios (0.3 for attack, 0.0001 for decay/release) that are fixed and not user-adjustable.

3. **State Machine**: The ADSR operates as a five-state machine:
   - **Idle**: Output is 0.0. Waiting for a gate-on event.
   - **Attack**: Output ramps exponentially toward 1.0. When it reaches 1.0, transitions to Decay.
   - **Decay**: Output falls exponentially toward the sustain level. When it reaches the sustain level, transitions to Sustain.
   - **Sustain**: Output holds at the sustain level indefinitely until a gate-off event.
   - **Release**: Output falls exponentially toward 0.0. When it reaches 0.0, transitions to Idle.

4. **Retrigger Behavior**: If the gate goes high again while the envelope is in the decay, sustain, or release phase, the envelope immediately re-enters the attack phase from its current output level. This means retriggering during release produces a smooth rise from wherever the envelope currently sits, rather than snapping back to zero first.

5. **Output Scaling**: The ADSR engine operates internally in the 0.0 to 1.0 range. The output is multiplied by 10.0 to produce the standard VCV Rack 0-10V unipolar CV range.

### Parameter Update Behavior

The attack, decay, sustain, and release parameters are synced from the UI module to the DSP module. When parameters change, the ADSR coefficients are recalculated using the `updateADSR()` method. Time parameters (attack, decay, release) are converted from seconds to sample counts, and the exponential coefficients are recomputed accordingly. The sustain level updates the decay target so that changes take effect on the next decay cycle.

## Tips

- For percussive sounds, use a short attack (0.001-0.01s), short decay (0.05-0.2s), zero sustain (0.0), and moderate release (0.1-0.3s). This creates a punchy transient that works well for controlling a VCA on drum or pluck patches.
- For pad-like swells, set a long attack (0.5-2.0s), moderate decay (0.5s), high sustain (0.8-1.0), and long release (1.0-4.0s). This produces smooth fade-ins and fade-outs suited to ambient textures.
- Patch the envelope output into a filter module's cutoff CV input to create classic subtractive synthesis filter sweeps. A fast attack with moderate decay and low sustain produces the characteristic "wah" effect on each note.
- Use multiple envelope modules with different ADSR settings to independently control amplitude and filter cutoff (or other parameters), giving each aspect of the sound its own contour.
- Because the envelope retriggers from its current level rather than resetting to zero, fast gate sequences produce overlapping envelope shapes that feel smooth and musical rather than choppy.
- The release parameter supports up to 4 seconds, which is twice the maximum attack and decay times. Use long release values with short gates to create sounds that ring out well beyond the gate duration, useful for reverb-like tails or cymbal-style decays.
- Connect the output of a StepTrig or Clock module to the GATE input for rhythmic envelope triggering synchronized to the sequencer.
