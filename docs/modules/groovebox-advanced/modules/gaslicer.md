# GASlicer

A breakbeat slicer that continuously loops a loaded audio sample and jumps to specific slice positions on trigger. The sample is divided into 16 equal slices, and incoming CV selects which slice to jump to when a trigger is received. Supports WAV, MP3, OGG, and FLAC files with automatic sample rate conversion. Inspired by classic breakbeat slicing techniques for rearranging drum loops and rhythmic audio in real time.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Trigger input for jumping to a slice position. A rising edge (crossing above 1.0V) causes playback to jump to the slice selected by the SLICE CV input. |
| 1 | SLICE | Control (CV) | Slice selection CV input. 0-10V maps to slices 0-15, selecting which of the 16 equal divisions of the sample to jump to on the next trigger. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | L | Audio | Left channel audio output, scaled and clamped to +/-5V. |
| 3 | R | Audio | Right channel audio output, scaled and clamped to +/-5V. For mono samples, this outputs the same signal as the L output. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| Sample | Text (read-only) | -- | -- | Displays the filename of the currently loaded sample. |
| Load Sample | Button | -- | -- | Opens a file browser to select an audio file (WAV, MP3, OGG, FLAC). The sample is loaded into memory and available for playback immediately. |

## Details

### Signal Flow

The module loads an audio sample into memory and immediately begins looping through it from start to finish. Audio is read from the sample buffer using linear interpolation between adjacent samples for smooth playback at non-integer positions. The interpolated output passes through a declick crossfade filter, is then scaled by 5.0, and hard-clamped to +/-5V before being sent to the L and R outputs.

### Continuous Looping Playback

Unlike a one-shot sampler, GASlicer plays continuously. Once a sample is loaded, playback loops from the beginning to the end of the sample and wraps back to the start indefinitely. This means audio is always being output as long as a sample is loaded, regardless of whether triggers are received.

### Trigger and Slice Selection

When a rising edge is detected on the TRIG input (voltage crossing above 1.0V), the module reads the SLICE CV input and jumps playback to the corresponding slice position. The slice is calculated as:

```
targetSlice = clamp(floor(sliceCV / 10.0 * 16), 0, 15)
playbackPosition = targetSlice * (sampleLength / 16)
```

The SLICE CV input is clamped to 0-10V. A voltage of 0V selects slice 0 (the beginning of the sample), while approximately 9.375V or higher selects slice 15 (the last sixteenth of the sample). After jumping, playback continues forward from the new position through the remainder of the sample, looping back to the beginning when it reaches the end.

### Declick Filter

To prevent audible clicks when the playback position jumps (either from a trigger or from the loop wrapping around), the module applies a crossfade filter. When a jump occurs, the output crossfades from the last audio value before the jump to the new audio at the jumped-to position. The crossfade ramps up over approximately 2048 samples (about 46ms at 44.1kHz), smoothly transitioning between the old and new audio to eliminate discontinuities.

### Sample Rate Conversion

The module automatically adjusts playback speed to account for differences between the loaded sample's native sample rate and the host's sample rate. The playback speed is calculated as:

```
playbackSpeed = fileSampleRate / hostSampleRate
```

This ensures the sample plays at its original pitch regardless of the host sample rate. There is no pitch control -- the sample always plays at its native pitch.

### Sample Loading

Samples are loaded into memory as separate left and right float buffers. Mono files are duplicated to both channels. The file's sample rate is recorded and used to compute the playback speed for correct pitch at the host's sample rate. Supported formats are WAV, MP3, OGG, and FLAC.

## Tips

- Load a drum break and use a step sequencer to send different SLICE CV values on each beat, rearranging the break into new patterns in classic breakbeat style.
- Pair with a GAClock or external clock sending triggers at the original tempo of the loop to keep sliced playback rhythmically aligned.
- Use a random or sample-and-hold module to generate random SLICE CV values for glitchy, unpredictable rearrangements of a drum loop.
- Since the module always loops, you can use it without any triggers as a simple continuous sample looper -- just load a sample and it plays.
- Send triggers at faster divisions than the original loop tempo to create rapid-fire stutter and roll effects on individual slices.
- Combine multiple GASlicer modules loaded with different breakbeats, triggered by the same clock but with different slice CV sources, for layered polyrhythmic breakbeat textures.
- Use a quantized CV source for the SLICE input to ensure clean jumps to exact slice boundaries, or use unquantized CV for more experimental, off-grid slice positions.
- For best results, use samples that divide cleanly into 16 equal parts (such as 4-bar or 8-bar loops at even tempos), since the slicing is always into 16 equal divisions regardless of the source material.
