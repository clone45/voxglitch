# GAResample

A stereo audio recorder that captures incoming audio to a shared buffer when triggered. It records for a configurable duration (20ms to 2 seconds) and writes to a shared buffer that the companion GAReplay module reads from. Use it to capture loops, one-shots, or snippets of any audio source within a GrooveboxAdvanced patch for later playback and manipulation.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Rising edge (above 1V) starts or restarts recording from the beginning of the buffer. A trigger received while already recording restarts recording from sample zero. |
| 1 | L | Audio | Left channel audio input. The signal is normalized from +/-5V to +/-1.0 before being written to the shared buffer. |
| 2 | R | Audio | Right channel audio input. The signal is normalized from +/-5V to +/-1.0 before being written to the shared buffer. |

## Outputs

None.

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| LENGTH | Knob | 0.0 - 1.0 | 0.5 | Controls the recording duration. The knob linearly maps 0.0 to 20ms and 1.0 to 2 seconds. At the default of 0.5, the recording length is approximately 1.01 seconds. The actual sample count is clamped to the maximum shared buffer size. |

## Details

### Signal Flow

GAResample is a write-only module with no audio outputs. It receives a stereo audio signal on its L and R inputs and records it into a shared buffer that is also accessible by GAReplay modules in the same patch. The buffer is allocated and managed by the GAProcessor, which passes shared pointers to all GAResample and GAReplay DSP instances.

### Recording Behavior

On each DSP sample, the module checks the TRIG input for a rising edge (signal crossing above 1V). When a rising edge is detected, recording begins (or restarts) from position zero in the shared buffer. While recording, each sample from the L and R inputs is divided by 5.0 to normalize from the +/-5V audio range to +/-1.0, and written to the corresponding position in the shared stereo buffers. The shared buffer length counter is updated on every sample so that a GAReplay module can see how much audio has been recorded so far.

Recording stops automatically when the write position reaches the target length (set by the LENGTH knob) or the maximum buffer size, whichever is smaller. If a new trigger arrives during recording, the write position resets to zero and recording begins again from the start, overwriting the previous contents.

### Length Calculation

The LENGTH knob maps linearly from 20 milliseconds (0.0) to 2 seconds (1.0) using the formula:

```
seconds = 0.020 + LENGTH * (2.0 - 0.020)
recordLength = seconds * sampleRate
```

At the standard 44100 Hz sample rate:
- LENGTH = 0.0: ~882 samples (20ms)
- LENGTH = 0.5: ~44,622 samples (~1.01s)
- LENGTH = 1.0: ~88,200 samples (2s)

### Normalization

Audio is normalized on write (divided by 5.0) so that the shared buffer stores values in the +/-1.0 range. The companion GAReplay module reverses this by multiplying by 5.0 on output. This normalization ensures consistent signal levels regardless of the source.

## Tips

- Pair GAResample with one or more GAReplay modules to create a record-and-playback loop. GAResample captures the audio; GAReplay plays it back with pitch and start-position control.
- Use a clock or rhythmic trigger to re-record at regular intervals, creating continuously evolving buffer content that the GAReplay module can mangle.
- Feed the output of an effects chain (e.g., from an EffectsOut or Mixer) into the L/R inputs to capture processed audio for further resampling and layering.
- Set LENGTH to short values (near 0.0) to capture percussive transients or micro-loops. Set it near 1.0 for longer phrases or ambient textures.
- Since a trigger during recording restarts from position zero, you can use fast retriggering to capture only the very beginning of a sound by sending a trigger shortly after the initial one.
- Multiple GAReplay modules can read from the same shared buffer simultaneously, each with different pitch and start-position settings, enabling polyphonic or granular-style playback from a single recording.
