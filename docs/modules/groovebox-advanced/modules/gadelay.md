# GADelay

A delay effect module that mixes a time-delayed copy of the input signal with the original dry signal. Supports adjustable delay time up to 1 second (or 0.5 seconds in short range mode), feedback for repeating echoes, and a dry/wet mix control. Useful for echo effects, slapback delays, rhythmic repeats, and ambient textures.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | Audio input signal to be delayed and processed. |
| 1 | TIME | Control (CV) | Delay time CV modulation. A bipolar +/-5V signal modulates the delay time by +/-50% relative to the TIME knob setting. For example, at +5V the effective delay time is 1.5x the knob value; at -5V it is 0.5x the knob value. The resulting effective time is clamped between 0.001s and the maximum time set by the RANGE switch (0.5s or 1.0s). |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | Mono audio output. The signal is a crossfade between the dry input and the delayed signal, controlled by the MIX parameter. Output is clamped to +/-5V. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| TIME | Knob | 0.01 - 1.0 | 0.25 | Delay time as a fraction of the maximum delay length. The actual delay time in seconds depends on the RANGE switch: in Long mode, this maps to 0.01-1.0 seconds; in Short mode, it maps to 0.005-0.5 seconds. |
| FDBK | Knob | 0.0 - 0.99 | 0.3 | Feedback amount. Controls how much of the delayed signal is fed back into the delay buffer along with the incoming audio. Higher values produce more echo repeats with slower decay. At 0.0, only a single echo is produced. Values approaching 0.99 produce very long, slowly decaying echo trails. |
| MIX | Knob | 0.0 - 1.0 | 0.5 | Dry/wet mix. At 0.0 only the dry input is heard; at 1.0 only the delayed (wet) signal is heard. At 0.5 (default) the dry and wet signals are mixed equally. The crossfade is linear: output = input * (1 - MIX) + delayed * MIX. |
| RANGE | Switch | Long / Short | Long | Selects the maximum delay time range. Long mode allows delays up to 1.0 second. Short mode limits the maximum to 0.5 seconds, which effectively doubles the resolution of the TIME knob across a shorter range for finer control over shorter delay times. |

## Details

### Signal Flow

1. **Input**: The audio signal is read from the IN port.

2. **Delay Time Calculation**: The effective delay time is computed by combining the TIME knob value with the TIME CV input. The knob value (0.01-1.0) is multiplied by the maximum time (1.0s in Long mode, 0.5s in Short mode) to produce a base delay time in seconds. The CV input is scaled from +/-5V to a +/-1 range, then applied as +/-50% proportional modulation of the base time. The result is clamped between 0.001s and the maximum time, then converted to a sample count based on the current sample rate. The internal buffer supports up to 65536 samples, which accommodates 1 second of delay at sample rates up to 65.5 kHz.

3. **Interpolated Delay Read**: The delayed sample is read from a circular buffer using linear interpolation between two adjacent samples. This interpolation ensures smooth, artifact-free behavior when the delay time is not an exact integer number of samples, and when modulating the delay time via the TIME CV input.

4. **Feedback Path**: The value written into the delay buffer at each sample is `input + delayed * feedback`. This means the delayed signal is attenuated by the feedback amount on each pass through the buffer. With feedback at 0.0, only the original input enters the buffer and a single echo is produced. With feedback at 0.99, each echo retains 99% of the previous echo's amplitude, producing a long trail of slowly decaying repeats.

5. **Dry/Wet Mix**: The output is a linear crossfade between the dry input signal and the wet delayed signal, controlled by the MIX parameter. This is computed as `output = input * (1 - mix) + delayed * mix`.

6. **Output Clamping**: The final output is clamped to the +/-5V range to prevent excessively loud signals from high feedback settings or hot inputs.

### CV Modulation Behavior

The TIME CV input provides proportional modulation relative to the knob setting. This means:
- At small TIME knob values, the same CV voltage produces a smaller absolute change in delay time.
- At large TIME knob values, the same CV voltage produces a larger absolute change.

This proportional scaling preserves the perceived modulation depth regardless of the base delay time setting. For pitch-based effects (like vibrato or chorus), this means the modulation sounds consistent across different delay time settings.

### Buffer Behavior

The circular buffer length is dynamically set based on the sample rate and the RANGE switch selection. In Long mode the buffer length is `sampleRate * 1.0` samples; in Short mode it is `sampleRate * 0.5` samples (both capped at 65536). The write position wraps around the buffer length, overwriting the oldest samples. Calling reset clears the entire buffer to silence.

## Tips

- For a classic slapback echo (rockabilly, dub), set TIME to a low value (0.05-0.15), FDBK to 0.0-0.2, and MIX to around 0.3-0.4. This produces a single tight repeat behind the original sound.
- For ambient, spacious echoes, use a longer TIME (0.4-0.8), moderate FDBK (0.5-0.7), and a MIX around 0.4. The repeats will trail off gradually, filling out the sonic space.
- Modulate the TIME input with a slow LFO to create chorus and vibrato effects. Use short delay times (switch to Short range for finer control) with a gentle triangle or sine LFO. The interpolated read ensures smooth pitch modulation without zipper noise.
- For rhythmic delay effects, set the TIME knob to match a subdivision of your song's tempo. With FDBK around 0.4-0.6, each echo will land on beat divisions, creating a rhythmic bounce.
- Use high feedback values (0.85-0.99) with caution for creative sound design. While the output is clamped to +/-5V and feedback is limited to below 1.0 (preventing true self-oscillation), high feedback creates very long echo trails that can build up in density and volume before eventually decaying.
- Chain GADelay with a GAFilter module in the feedback path (by routing through the filter before re-entering the delay) to create filtered delay effects where each echo repeat becomes progressively darker or brighter.
- In Short range mode, the TIME knob covers half the time range, giving you twice the precision for dialing in short delays. This is useful for doubling effects, tight slapback, and chorus where small time differences matter.
- Set MIX to 1.0 (fully wet) when using GADelay in a parallel effects chain with a GAMixer, so you can control the dry/wet balance at the mixer level instead.
