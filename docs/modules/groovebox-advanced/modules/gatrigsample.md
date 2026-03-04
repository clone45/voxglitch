# GATrigSample

A triggered sample player that plays a loaded audio file from a configurable start position each time it receives a trigger. Supports V/OCT pitch control for chromatic playback and CV-controllable start position for sample slicing. Loads WAV, MP3, OGG, and FLAC files with automatic sample rate conversion, making it well suited for drum hits, one-shot sound effects, vocal chops, and pitched sample playback.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Trigger input for starting sample playback. A rising edge (crossing above 1.0V) restarts playback from the current start position, cutting off any in-progress playback. |
| 1 | V/OCT | Control (CV) | Pitch control following the 1V/octave standard. Added to the PITCH knob value before conversion to a playback speed multiplier via 2^V. 0V plays at original pitch, +1V doubles speed (one octave up), -1V halves speed (one octave down). |
| 2 | STRT | Control (CV) | Start position CV input. 0-10V maps to 0.0-1.0 (beginning to end of sample), added to the START knob value. The combined value is clamped to 0.0-1.0. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | L | Audio | Left channel audio output, scaled and clamped to +/-5V. |
| 4 | R | Audio | Right channel audio output, scaled and clamped to +/-5V. For mono samples, this outputs the same signal as the L output. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| PITCH | Knob | -2.0 to +2.0 | 0.0 | Pitch offset in V/OCT units. At 0.0, the sample plays at its original pitch (adjusted for sample rate differences). +1.0 doubles the speed (one octave up), -1.0 halves it (one octave down). Added to the V/OCT input before computing the playback speed multiplier. |
| START | Knob | 0.0 to 1.0 | 0.0 | Base start position as a fraction of the sample length. 0.0 is the beginning, 1.0 is the end. Added to the STRT CV input (divided by 10) to determine where playback begins on each trigger. |
| Sample | Text (read-only) | -- | -- | Displays the filename of the currently loaded sample. |
| Load Sample | Button | -- | -- | Opens a file browser to select an audio file (WAV, MP3, OGG, FLAC). The sample is loaded into memory and available for playback immediately. |

## Details

### Signal Flow

When a trigger is received, the module calculates a start position from the START knob and STRT CV input, then begins playing the loaded sample from that point forward. Audio is read from the sample buffer using linear interpolation between adjacent samples for smooth playback at non-integer positions. The interpolated output is scaled by 5.0 and hard-clamped to +/-5V before being sent to the L and R outputs.

### Trigger Detection

The module detects rising edges on the TRIG input by monitoring when the voltage crosses above 1.0V. Each rising edge restarts playback from the calculated start position, even if the previous playback has not yet finished. Playback continues until the end of the sample is reached, at which point the module goes silent and outputs 0V on both channels.

### Start Position

On each trigger, the start position is computed as:

```
startFraction = clamp(START_knob + (STRT_CV / 10.0), 0.0, 1.0)
startSample = startFraction * sampleLength
```

The STRT CV input uses a 0-10V range, where 10V corresponds to the end of the sample. This is added to the START knob value (already in the 0.0-1.0 range), and the sum is clamped. This means the knob sets a base position and the CV can shift it forward, or both can be used together to span the full sample.

### Pitch and Playback Speed

The playback speed is determined by three factors:

1. **Sample rate conversion**: A base speed ratio of `fileSampleRate / hostSampleRate` ensures the sample plays at its correct pitch regardless of the host sample rate.
2. **V/OCT input**: The voltage from the V/OCT port.
3. **PITCH knob**: An offset added to the V/OCT input.

These combine as:

```
playbackSpeed = baseSpeed * 2^(V/OCT + PITCH)
```

The combined V/OCT value is clamped to +/-10V before the exponential conversion, giving a theoretical pitch range of 20 octaves. The exponential conversion uses a fast polynomial approximation of 2^x (Taylor series based) for efficient per-sample computation. With the PITCH knob range of +/-2 and no CV, the sample can be pitched 2 octaves up or down from its original pitch.

### Sample Loading

Samples are loaded into memory as separate left and right float buffers. Mono files are duplicated to both channels. The file's sample rate is recorded and used to compute the base playback speed for correct pitch at the host's sample rate. Supported formats are WAV, MP3, OGG, and FLAC. When a new sample path is set, loading occurs during the next sync cycle from the UI module to the DSP module; the DSP module tracks the currently loaded path and only reloads when the path changes.

### Linear Interpolation

The module reads samples using linear interpolation to produce smooth output at fractional playback positions. Given a position between two integer sample indices, the output is a weighted blend of the two surrounding samples. This avoids the aliasing and stepping artifacts that would result from nearest-neighbor lookup, which is especially important when the playback speed differs from 1.0 due to pitch shifting or sample rate conversion.

### One-Shot Behavior

This is a one-shot player: playback starts on trigger and runs to the end of the sample, then stops. There is no looping. Retriggering before the sample finishes will immediately restart playback from the new start position, cutting off any remaining audio from the previous trigger.

## Tips

- Use the START knob and STRT CV to slice through a drum break or long sample, triggering different segments on each hit for beat-juggling effects.
- Connect a sequencer's V/OCT output to the V/OCT input to play chromatic melodies with a tonal sample such as a single-note instrument recording.
- Set the PITCH knob to -1.0 or -2.0 to slow down a percussive hit for pitched-down, heavy percussion textures.
- For layered drum kits, use multiple GATrigSample modules triggered by different pattern generators, each loaded with a different sample (kick, snare, hi-hat, etc.).
- Modulate the STRT input with an LFO while retriggering rapidly with a fast clock to create granular-style texture scanning through a longer sample.
- Feed a random voltage source into the STRT CV to pick random starting positions on each trigger, producing unpredictable sample slice variations from a single audio file.
- Load a noise or texture sample and use high START values to isolate the tail, creating short transient bursts useful for hi-hat or shaker sounds.
- Combine the PITCH knob with V/OCT CV from a quantizer module to ensure pitch-shifted playback stays in a musical scale.
