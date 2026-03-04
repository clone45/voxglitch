# GASampler

A triggered sample player with position control and V/OCT pitch tracking. On receiving a trigger, playback starts from a configurable position within the loaded audio file and plays through to the end of the sample. Supports WAV, MP3, OGG, and FLAC files with automatic sample rate conversion, making it suitable for percussive one-shots, vocal chops, and textural sample playback.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Trigger input for starting sample playback. A rising edge (crossing above 1.0V) restarts playback from the current start position. |
| 1 | V/OCT | Control (CV) | Pitch control following the 1V/octave standard. Added to the PITCH knob value before conversion to a playback speed multiplier via 2^V. |
| 2 | POS | Control (CV) | Start position CV input. 0-10V maps to 0.0-1.0 (beginning to end of sample), added to the POS knob value. The combined value is clamped to 0.0-1.0. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | L | Audio | Left channel audio output, scaled and clamped to +/-5V. |
| 4 | R | Audio | Right channel audio output, scaled and clamped to +/-5V. For mono samples, this outputs the same signal as the L output. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| PITCH | Knob | -2.0 to +2.0 | 0.0 | Pitch offset in V/OCT units. At 0.0, the sample plays at its original pitch (adjusted for sample rate differences). +1.0 doubles the speed (one octave up), -1.0 halves it (one octave down). Added to the V/OCT input before computing the playback speed multiplier. |
| POS | Knob | 0.0 to 1.0 | 0.0 | Base start position as a fraction of the sample length. 0.0 is the beginning, 1.0 is the end. Added to the POS CV input (divided by 10) to determine where playback begins on each trigger. |
| Sample | Text (read-only) | -- | -- | Displays the filename of the currently loaded sample. |
| Load Sample | Button | -- | -- | Opens a file browser to select an audio file (WAV, MP3, OGG, FLAC). The sample is loaded into memory and available for playback immediately. |

## Details

### Signal Flow

When a trigger is received, the module calculates a start position from the POS knob and POS CV input, then begins playing the loaded sample from that point forward. Audio is read from the sample buffer using linear interpolation between adjacent samples for smooth playback at non-integer positions. The interpolated output is scaled by 5.0 and hard-clamped to +/-5V before being sent to the L and R outputs.

### Trigger Detection

The module detects rising edges on the TRIG input by monitoring when the voltage crosses above 1.0V. Each rising edge restarts playback from the calculated start position, even if the previous playback has not yet finished. Playback continues until the end of the sample is reached, at which point the module goes silent and outputs 0V on both channels.

### Start Position

On each trigger, the start position is computed as:

```
startFraction = clamp(POS_knob + (POS_CV / 10.0), 0.0, 1.0)
startSample = startFraction * sampleLength
```

The POS CV input uses a 0-10V range, where 10V corresponds to the end of the sample. This is added to the POS knob value (already in the 0.0-1.0 range), and the sum is clamped. This means the knob sets a base position and the CV can shift it forward, or both can be used together to span the full sample.

### Pitch and Playback Speed

The playback speed is determined by three factors:

1. **Sample rate conversion**: A base speed ratio of `fileSampleRate / hostSampleRate` ensures the sample plays at its correct pitch regardless of the host sample rate.
2. **V/OCT input**: The voltage from the V/OCT port.
3. **PITCH knob**: An offset added to the V/OCT input.

These combine as:

```
playbackSpeed = baseSpeed * 2^(V/OCT + PITCH)
```

The combined V/OCT value is clamped to +/-10V before the exponential conversion, giving a theoretical pitch range of 20 octaves. With the PITCH knob range of +/-2 and no CV, the sample can be pitched 2 octaves up or down from its original pitch.

### Sample Loading

Samples are loaded into memory as separate left and right float buffers. Mono files are duplicated to both channels. The file's sample rate is recorded and used to compute the base playback speed for correct pitch at the host's sample rate. Supported formats are WAV, MP3, OGG, and FLAC.

### One-Shot Behavior

This is a one-shot player: playback starts on trigger and runs to the end of the sample, then stops. There is no looping. Retriggering before the sample finishes will immediately restart playback from the new start position, cutting off any remaining audio from the previous trigger.

## Tips

- Use the POS knob and CV to scrub through different starting points in a drum break or long sample, creating variations on each trigger.
- Send a sequencer or pattern generator CV into the POS input to create chopped-up, rearranged playback of a longer sample.
- Connect a V/OCT source from a sequencer to play melodic phrases with a tonal sample (a single-cycle waveform or a short vocal snippet).
- Set the PITCH knob to -1.0 or -2.0 to slow down a drum hit for pitched-down, heavy percussion effects.
- For layered percussion, use multiple GASampler modules triggered by the same clock but loaded with different samples (kick, snare, hi-hat).
- Modulate the POS input with an LFO for granular-style texture scanning through a longer sample, retriggering rapidly with a clock or trigger source.
- Load a noise or texture sample and use high POS values to start near the tail, creating short transient bursts useful for hi-hats or shaker sounds.
