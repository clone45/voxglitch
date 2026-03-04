# GATrigSampleFX

A triggered sample player with six per-sample audio effects designed for sound design and rhythmic mangling. On receiving a trigger, the module plays back a loaded audio file while applying a selectable effect (Micro-Loop, Comb Resonance, Echo, Decimate, Stutter, Reverse, Buildup, or Falldown) to the body and tail of the sound. The initial transient/attack portion of the sample is preserved clean, with the chosen effect engaging only after a configurable transient window. Supports WAV, MP3, OGG, and FLAC files with automatic sample rate conversion.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Trigger input for starting sample playback. A rising edge (crossing above 1.0V) restarts playback from the beginning of the sample, resets the transient window, and reinitializes the selected effect. |
| 1 | V/OCT | Control (CV) | Pitch control following the 1V/octave standard. Converted to a playback speed multiplier via 2^V and multiplied with the base playback speed. Clamped to +/-10V before conversion. |
| 2 | AMT | Control (CV) | Effect amount CV modulation. A 0-10V signal is divided by 10 and added to the AMOUNT knob value, with the result clamped to 0.0-1.0. Modulated at audio rate for dynamic effect control. |
| 3 | TRANS | Control (CV) | Transient duration CV modulation. A 0-10V signal is divided by 10 and added to the TRANSIENT knob value, with the result clamped to 0.0-1.0. Modulated at audio rate. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 4 | L | Audio | Left channel audio output, scaled and clamped to +/-5V. |
| 5 | R | Audio | Right channel audio output, scaled and clamped to +/-5V. For mono samples, this outputs the same signal as the L output. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| Sample | Text (read-only) | -- | -- | Displays the filename of the currently loaded sample. |
| Load Sample | Button | -- | -- | Opens a file browser to select an audio file (WAV, MP3, OGG, FLAC). The sample is loaded into memory and available for playback immediately. |
| FX | Dropdown | None, MLoop, Comb, Echo, Decimate, Stutter, Reverse, Buildup, Falldown | None | Selects which effect is applied to the body/tail of the sample after the transient window. |
| AMOUNT | Knob | 0.0 to 1.0 | 0.5 | Controls the intensity or character of the selected effect. The exact behavior depends on the chosen effect type (see Details). Added to the AMT CV input before clamping. |
| MIX | Knob | 0.0 to 1.0 | 1.0 | Wet/dry mix. At 0.0, only the dry (unprocessed) signal is output. At 1.0, only the wet (effected) signal is output. Intermediate values crossfade between the two as: output = dry * (1 - mix) + wet * mix. |
| TRANSIENT | Knob | 0.0 to 1.0 | 0.3 | Controls the duration of the clean transient window at the start of each triggered playback. Maps linearly to 0-50ms. During this window, the wet signal equals the dry signal, preserving the attack of the sample regardless of the chosen effect. Also used by Buildup and Falldown to control initial/starting slice size. |

## Details

### Signal Flow

When a trigger is received, the module begins forward playback of the loaded sample. Two parallel signals are generated each sample:

1. **Dry signal**: The sample played back normally at the computed playback speed with linear interpolation.
2. **Wet signal**: During the transient window, this equals the dry signal. After the transient window expires, the selected effect algorithm processes or replaces the signal.

The final output is a crossfade between dry and wet controlled by the MIX parameter, then scaled by 5.0 and hard-clamped to +/-5V.

### Transient Preservation

On each trigger, the module computes a transient duration in samples from the TRANSIENT knob (plus TRANS CV):

```
transientMs = effectiveTransient * 50.0
transientSamples = transientMs * sampleRate / 1000.0
```

For the first `transientSamples` samples after a trigger, the wet signal is set equal to the dry signal. This means the attack portion of drum hits and percussive sounds passes through cleanly, and the effect only engages on the sustain/tail. At the default TRANSIENT value of 0.3, this preserves approximately 15ms of the attack.

### Trigger Detection

The module detects rising edges on the TRIG input by monitoring when the voltage crosses above 1.0V. Each rising edge restarts playback from the beginning, recomputes the transient window, and reinitializes all effect state. Retriggering before the sample finishes immediately restarts playback.

### Pitch and Playback Speed

The playback speed combines sample rate conversion with V/OCT pitch control:

```
playbackSpeed = (fileSampleRate / hostSampleRate) * 2^(V/OCT)
```

The V/OCT input is clamped to +/-10V before exponential conversion, giving a theoretical range of 20 octaves. With no CV connected, the sample plays at its original pitch (adjusted for any difference between the file's sample rate and the host sample rate).

### Effect Types

#### None
No effect is applied. The wet signal equals the dry signal at all times.

#### MLoop (Micro-Loop)
Creates a buzzing, granular texture by repeatedly looping a small segment of the sample. The AMOUNT parameter controls the loop size on an exponential scale: at 0.0 the loop is approximately 50ms (low buzz), and at 1.0 it shrinks to approximately 0.5ms (high-pitched zap). The loop window slowly scans forward through the sample at 5% of the normal playback speed, creating a time-stretched effect that is roughly 20x slower than normal playback.

#### Comb
A feed-forward comb filter that creates metallic, resonant coloring. The delay time ranges from approximately 0.04ms to 10ms (2 to 480 samples at 48kHz), controlled by AMOUNT. Feedback ranges from 0.7 (at AMOUNT 0.0) to 0.92 (at AMOUNT 1.0). Lower AMOUNT values produce lower-pitched resonances; higher values produce higher-pitched, more metallic tones.

#### Echo
A delay-based echo effect with longer delay times than Comb. The delay ranges from approximately 10ms to 200ms (480 to 9600 samples at 48kHz), controlled by AMOUNT. Feedback ranges from 0.8 to 0.95. At low AMOUNT values this produces a subtle doubling; at high values it creates rhythmic echo repeats that can self-oscillate.

#### Decimate
A sample-rate reduction (bit-crushing style) effect. AMOUNT controls the hold period: at 0.0 the signal updates every sample (no effect), at 1.0 it holds each sample value for 100 samples before updating, creating a heavily aliased, lo-fi sound. The effect operates by freezing the output at the last captured sample value until the hold counter expires.

#### Stutter
A retriggering effect that repeatedly plays a shrinking chunk of audio, creating a rapid-fire stuttering that accelerates and fades out. On trigger, the initial chunk length is set by AMOUNT (5ms to 100ms at 44.1kHz). After each repetition, the chunk shrinks by 28% and the amplitude decays by 15%. Playback stops when the chunk length falls below 8 samples or the amplitude drops below 1%. The stutter begins at the position where the transient window ends.

#### Reverse
Plays the sample backwards from the end. The AMOUNT parameter controls reverse playback speed: at 0.0 the reverse speed is 0.5x normal, at 1.0 it is 3.0x normal. Once the reverse position reaches the beginning of the sample, the output goes silent.

#### Buildup
A progressive reveal effect that starts by looping a tiny slice of the sample, then doubles the slice length after a set number of repetitions, gradually exposing more and more of the sample until the full sample plays through once. The TRANSIENT parameter controls the initial slice size on an exponential curve: at 0.0 the initial slice is approximately 0.05% of the sample (many doublings needed, long buildup), at 1.0 it starts at 50% (just one doubling before full playback). The AMOUNT parameter controls how many times each slice repeats before doubling, from 1 (quick reveal) to 32 (very slow, hypnotic buildup). After the slice length reaches the full sample length, the effect transitions to a single complete playthrough before stopping.

#### Falldown
The inverse of Buildup: starts by looping a larger slice of the sample, then halves the slice length after a set number of repetitions, collapsing the audio into an increasingly tiny, rapid loop until it reaches a minimum size (32 samples) and stops. The TRANSIENT parameter controls the starting slice size on an exponential curve: at 0.0 the starting slice is 50% of the sample (many halvings, long collapse), at 1.0 it starts at approximately 0.05% (already tiny, quick stop). The AMOUNT parameter controls repetitions per slice, from 1 (quick collapse) to 32 (slow, drawn-out decay).

### One-Shot Behavior

For most effects, playback stops when the dry position reaches the end of the sample. The Buildup and Falldown effects manage their own lifecycle and may continue playing after the dry sample has finished: Buildup continues until its final full playthrough completes, and Falldown continues until the slice size reaches the minimum threshold of 32 samples.

### Sample Loading

Samples are loaded into memory as separate left and right float buffers. Mono files are duplicated to both channels. The file's sample rate is recorded and used to compute the base playback speed for correct pitch at the host's sample rate. Supported formats are WAV, MP3, OGG, and FLAC.

## Tips

- Start with the TRANSIENT knob at 0.3-0.5 to preserve the punch of drum hits while mangling the tail. Increase it further for sounds with longer attacks (pads, swells) to keep the onset clean.
- Use the MIX knob at intermediate values (0.3-0.7) to blend the dry sample with the effect, keeping the original character present while adding texture.
- The Comb effect at low AMOUNT values adds subtle metallic coloring to hi-hats and cymbals. At higher values, it creates pitched resonances that can make percussive sounds more tonal.
- MLoop is effective for creating glitchy, granular textures from any source material. Modulate the AMT input with an LFO to sweep the loop size for evolving buzzy tones.
- Stutter works well on drum hits for classic rapid-fire snare rolls and build-up effects. Use higher AMOUNT values for longer initial chunks that create more obvious stuttering.
- Apply Reverse to cymbals and crash samples for classic reversed cymbal swells. Use AMOUNT around 0.5-0.7 for a natural-sounding reverse speed.
- Echo at moderate AMOUNT values (0.3-0.5) adds rhythmic depth to percussion. Be careful at high AMOUNT values where the high feedback can cause the echoes to build up.
- Decimate at subtle settings (AMOUNT 0.1-0.3) adds lo-fi grit to drum hits. At extreme settings it creates heavily aliased, retro-sounding textures.
- Buildup is ideal for creating tension and anticipation: load a vocal stab or drum hit, set AMOUNT high (0.7-1.0) for many repetitions per slice, and let the sound gradually reveal itself.
- Falldown creates a natural decay effect: pair it with a long sample and moderate AMOUNT to create a gradual rhythmic collapse from a full loop into a tiny grain.
- Modulate the AMT CV input with a step sequencer to change effect intensity on every beat, creating evolving patterns from a single sample and effect type.
- Pair with GATrigSample modules playing the same sample dry, and use GATrigSampleFX with MIX at 1.0 for a parallel wet-only signal that can be mixed separately.
