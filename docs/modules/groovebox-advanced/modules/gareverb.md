# GAReverb

A stereo plate reverb module implementing the Dattorro plate reverb algorithm. It produces lush, diffuse reverberation with adjustable decay time, high-frequency damping, and a dry/wet mix control. The stereo output is derived from decorrelated taps across both halves of a recirculating tank, producing a wide, immersive spatial image. Useful for adding ambience, space, and depth to any audio source.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | L | Audio | Left channel audio input. Combined with the R input into a mono sum before entering the reverb's input diffuser chain. Expects signals in the standard VCV Rack +/-5V audio range. |
| 1 | R | Audio | Right channel audio input. Combined with the L input into a mono sum before entering the reverb's input diffuser chain. Expects signals in the standard VCV Rack +/-5V audio range. |
| 2 | MIX | Control (CV) | Dry/wet mix CV modulation. A unipolar 0-10V signal that adds to the MIX knob value. The CV is scaled so that 0V adds nothing and 10V adds 0.5 to the mix. The resulting effective mix is clamped to the 0.0-1.0 range. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | L | Audio | Left channel audio output. A crossfade between the dry left input and the wet reverb left signal, controlled by the effective mix value. Output is clamped to +/-10V. |
| 4 | R | Audio | Right channel audio output. A crossfade between the dry right input and the wet reverb right signal, controlled by the effective mix value. Output is clamped to +/-10V. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| DECAY | Knob | 0.1 - 0.95 | 0.5 | Controls the reverb tail length by setting the decay multiplier applied to the signal each time it circulates through the tank. Lower values produce a short, tight reverb that dies away quickly. Higher values produce a long, sustained tail. At 0.95 the reverb sustains for a very long time. Also influences the decay diffusion 2 allpass coefficient, which is computed as decay + 0.15 (clamped to 0.25-0.70). |
| DAMP | Knob | 0.0 - 1.0 | 0.5 | Controls high-frequency damping inside the reverb tank. At 0.0, no damping is applied and the reverb retains full brightness on every recirculation. At 1.0, maximum damping is applied and high frequencies are heavily attenuated on each pass through the tank, producing a dark, warm reverb tail. Internally, the damping coefficient for the one-pole lowpass filter is computed as 1.0 - (DAMP * 0.9995). |
| MIX | Knob | 0.0 - 1.0 | 0.35 | Dry/wet mix balance. At 0.0, only the dry input is heard. At 1.0, only the reverb (wet) signal is heard. The crossfade is linear per channel: outputL = inputL * (1 - mix) + wetL * mix, and likewise for the right channel. Can be further modulated by the MIX CV input. |

## Details

### Algorithm

GAReverb implements the Dattorro plate reverb, based on Jon Dattorro's 1997 paper "Effect Design Part 1: Reverberator and Other Filters." This is a widely respected reverb topology known for producing smooth, dense, and musically pleasing reverberation. All internal delay line lengths are defined relative to a 29761 Hz reference sample rate and are automatically rescaled to match the host sample rate, ensuring consistent reverb character across different sample rates. The delay buffers are sized to support sample rates up to 192 kHz.

### Signal Flow

1. **Input Summing**: The left and right input signals are summed to mono, then scaled by 0.1 to normalize from VCV Rack's +/-10V (combined L+R) range to an internal +/-1.0 working range.

2. **Input Bandwidth Filter**: A one-pole lowpass filter with a coefficient of 0.9995 gently rolls off the very highest frequencies of the input signal before it enters the diffusion chain. This prevents harshness in the reverb tail.

3. **Input Diffusion**: The signal passes through four series allpass filters (with delay lengths of 142, 107, 379, and 277 samples at 29761 Hz). The first two use a diffusion coefficient of 0.75 and the second two use 0.625. These smear the transients of the input signal into a diffuse wash, which is essential for the dense, smooth reverb sound.

4. **Tank Recirculation**: The diffused signal enters two parallel tank halves. Each tank half receives the diffused input signal plus cross-feedback from the opposite tank half's post-damping delay output, multiplied by the decay coefficient. This cross-coupled structure creates the continuous recirculation that sustains the reverb tail.

5. **Tank Processing (per half)**: Each tank half processes the signal through the following chain:
   - **Decay Diffusion 1 Allpass**: An allpass filter with a negative coefficient of -0.70, whose delay length is modulated by a slow (~1 Hz) sine-wave LFO. The two tank halves use sine and cosine (90 degrees apart) for decorrelation. This modulation prevents metallic ringing and adds subtle motion to the reverb tail.
   - **Pre-Damping Delay**: A long delay line (4453 or 4217 samples at 29761 Hz) that provides the primary time spacing in the tank.
   - **Damping Lowpass**: A one-pole lowpass filter controlled by the DAMP parameter that progressively absorbs high frequencies on each recirculation, simulating the natural absorption of real acoustic spaces.
   - **Decay Scaling**: The signal is multiplied by the decay coefficient, attenuating the overall level on each pass.
   - **Decay Diffusion 2 Allpass**: An allpass filter with a positive coefficient (derived from decay + 0.15, clamped to 0.25-0.70) that further diffuses the signal within the tank.
   - **Post-Damping Delay**: Another long delay line (3720 or 3163 samples at 29761 Hz) whose output feeds back into the opposite tank half.

6. **Output Tapping**: The stereo output is constructed by summing seven taps from various points within both tank halves for each output channel. Some taps are added and some are subtracted, following Dattorro's prescribed tap positions and polarities. The left and right channels tap from different positions in the tank, creating a naturally decorrelated stereo image.

7. **DC Blocking**: Each output channel passes through a DC blocker (Julius O. Smith form with a cutoff around 5 Hz) to remove any DC offset that may accumulate in the recirculating tank, without attenuating audible bass frequencies.

8. **Output Scaling and Safety**: The tapped output is scaled by a factor of 3.0 (combining a 0.6 tap gain with a 5.0x VCV voltage scaling). A NaN/Inf safety check is applied -- if either output is non-finite, both outputs are zeroed and all internal buffers are cleared. The wet signal is then clamped to +/-10V.

9. **Dry/Wet Mix**: The final output is a linear crossfade between the original dry input (per channel) and the wet reverb signal, controlled by the effective mix value (MIX knob + MIX CV).

### CV Modulation Behavior

The MIX CV input provides additive modulation to the MIX knob value. The incoming 0-10V CV signal is scaled so that 10V adds 0.5 to the mix value. This means:
- With the MIX knob at its default of 0.35, a 10V CV brings the effective mix to 0.85.
- With the MIX knob at 0.5, a 10V CV brings the effective mix to 1.0 (fully wet).
- The effective mix is always clamped to the 0.0-1.0 range, so overdriving the CV will not cause distortion.

### Denormal Protection

The one-pole lowpass filters used for bandwidth and damping include denormal flushing (adding and subtracting a tiny constant of 1e-25) to prevent CPU spikes that can occur when the reverb tail decays to near-silence. This ensures the module remains CPU-efficient even when no audio is passing through.

## Tips

- For a short, tight room ambience, set DECAY to 0.2-0.3, DAMP to 0.6-0.8, and MIX to 0.15-0.25. This adds a subtle sense of space without washing out the original signal.
- For a large hall or cathedral effect, set DECAY to 0.8-0.95, DAMP to 0.3-0.5, and MIX to 0.3-0.5. The long tail with moderate damping produces a warm, expansive sound.
- For shimmering ambient pads, use high DECAY (0.85-0.95) with low DAMP (0.0-0.2) to let high frequencies sustain in the tank. This creates bright, ethereal reverb trails.
- For dark, moody reverb, set DAMP to 0.7-1.0 to aggressively filter highs on each recirculation. Even with moderate DECAY, the tail will sound warm and subdued.
- Use the MIX CV input with an envelope follower to create dynamic reverb: louder signals push the mix wetter, making accented hits ring out more while quieter passages stay drier.
- Modulate the MIX CV input with a slow LFO to create evolving spatial movement, where the reverb fades in and out rhythmically.
- For a fully wet reverb send, set MIX to 1.0 and use GAReverb in a parallel effects chain with a GAMixer. This lets you control the reverb level at the mixer while keeping the dry signal path separate.
- The stereo input is summed to mono internally before entering the reverb tank, but the outputs are true stereo derived from decorrelated tank taps. This means even a mono input will produce a wide stereo reverb field.
- Chain GAReverb after a GADelay module to create delay-into-reverb effects, where each echo repeat feeds into the reverb for increasingly diffuse repeats.
- The DECAY parameter's upper limit of 0.95 prevents the tank from self-oscillating or building up to dangerous levels, but values close to 0.95 can still produce very long, dense tails. Use these extreme settings intentionally for drone and ambient sound design.
