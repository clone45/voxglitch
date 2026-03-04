# GAComb

A comb filter module that creates resonant, pitched coloration by mixing the input signal with a short delayed copy of itself. Comb filters produce a series of evenly-spaced peaks and notches in the frequency spectrum, useful for metallic tones, Karplus-Strong-style plucked sounds, flanging effects, and resonant pitched textures.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | Audio input signal to be processed by the comb filter. |
| 1 | TIME | Control (CV) | Delay time CV modulation. A bipolar +/-5V signal modulates the delay time by +/-50% relative to the TIME knob setting. For example, at +5V the effective delay time is 1.5x the knob value; at -5V it is 0.5x the knob value. The resulting effective time is clamped to the 0.001s-0.05s range. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | Mono audio output. The signal is the sum of the dry input and the delayed signal scaled by the feedback amount. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| TIME | Knob | 0.001 - 0.05 s | 0.01 | Delay time in seconds. Controls the fundamental pitch of the comb filter's resonance. At 0.01s the comb resonance sits at 100 Hz; at the minimum 0.001s it reaches 1000 Hz; at the maximum 0.05s it drops to 20 Hz. Shorter times produce higher-pitched metallic tones, while longer times produce lower-pitched resonances. |
| FDBK | Knob | -0.98 - 0.98 | 0.5 | Feedback amount. Controls the amplitude of the delayed signal mixed into the output and fed back into the delay buffer. Positive values reinforce the comb peaks (producing a bright, ringing character). Negative values cancel even harmonics and reinforce odd harmonics, producing a hollow, clarinet-like timbre. Values closer to +/-0.98 produce longer decay times and sharper resonant peaks. At 0.0 the filter produces a single echo with no recirculation. |

## Details

### Signal Flow

1. **Input**: The audio input signal is read from the IN port.

2. **Delay Time Calculation**: The effective delay time is computed by combining the TIME knob value with the TIME CV input. The CV is scaled from +/-5V to a +/-1 range, then mapped to +/-50% modulation depth around the knob value. The result is clamped between 0.001s and 0.05s, then converted to a sample count based on the current sample rate. The maximum internal buffer size is 8192 samples, which supports delay times up to approximately 50ms at 96 kHz.

3. **Interpolated Delay Read**: The delayed sample is read from a circular buffer using linear interpolation between adjacent samples. This interpolation is important because the delay time in samples is typically not an integer, and without interpolation the pitch resolution would be coarse and produce audible stepping artifacts when modulating the time parameter.

4. **Comb Filter Topology**: The module implements a hybrid feedforward/feedback comb filter:
   - **Output**: `output = input + delayed * feedback` -- the dry signal is summed with the delayed signal scaled by the feedback amount.
   - **Buffer Write**: `buffer = input + delayed * feedback * 0.5` -- the value written back into the delay buffer includes a half-strength feedback term. This means the recirculation is attenuated compared to the initial feedforward path, producing a comb filter that has strong initial coloration but a controlled decay rather than infinite ringing.

5. **Safety**: If the output becomes non-finite (NaN or infinity), the output is set to 0 and the entire delay buffer is cleared. This prevents runaway feedback from producing sustained noise or silence-breaking artifacts.

### Frequency Response

A comb filter creates peaks at multiples of 1/delayTime Hz. For example, with a 10ms delay time (the default), peaks occur at 100 Hz, 200 Hz, 300 Hz, and so on. With positive feedback these peaks are reinforced; with negative feedback the peaks shift to odd multiples of 1/(2*delayTime) Hz (50 Hz, 150 Hz, 250 Hz, etc.).

### CV Modulation Behavior

The TIME CV input provides proportional modulation relative to the knob setting. This means:
- At small TIME knob values, the same CV voltage produces a smaller absolute change in delay time.
- At large TIME knob values, the same CV voltage produces a larger absolute change.

This proportional scaling is musically useful because it preserves the perceived pitch modulation depth regardless of the base delay time setting.

## Tips

- For Karplus-Strong plucked string synthesis, feed a short burst of noise or an impulse into IN, set a moderate positive FDBK (0.6-0.8), and tune the TIME knob to set the pitch of the pluck. The string pitch in Hz is approximately 1/TIME.
- Use negative feedback values to get a hollow, clarinet-like quality that emphasizes odd harmonics. This is particularly effective on sawtooth or noise sources.
- Modulate the TIME input with an LFO to create flanging and chorus-like effects. Slow, subtle modulation (a triangle LFO at 0.1-0.5 Hz) produces classic flanging; faster modulation creates vibrato-like pitch wobble.
- Chain two GAComb modules in series with slightly different delay times to create a denser, more complex resonant texture.
- Feed a drum sound through GAComb with short delay times (0.001-0.005s) and high feedback to add a metallic, pitched ring to percussion.
- At very high feedback values (above 0.9 or below -0.9), the comb filter rings for a long time after the input stops, turning any transient input into a sustained pitched tone. Use this as a resonator by exciting it with triggers or short bursts.
- The half-strength feedback in the buffer write path means the filter will not self-oscillate indefinitely, even at maximum feedback. This makes it safe to use high feedback values without worrying about runaway gain.
