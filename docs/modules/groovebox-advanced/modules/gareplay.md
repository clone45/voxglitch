# GAReplay

A stereo sample playback module that reads from the shared resample buffer written by GAResample. When triggered, it plays back the recorded audio with pitch control via a V/OCT input and a pitch offset knob, enabling pitched playback, time-stretching effects, and re-triggered sample phrases. Designed to work as the playback half of the Resample/Replay pair.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Starts playback on a rising edge (threshold > 1.0V). Each trigger restarts playback from the START position. |
| 1 | V/OCT | Control (CV) | Pitch control following the 1V/octave standard. 0V plays at original speed, +1V doubles speed (one octave up), -1V halves speed (one octave down). Added to the PITCH knob value before conversion. |
| 2 | START | Control (CV) | Sets the playback start position within the buffer. 0V starts at the beginning, 10V starts at the end. The value is read on each trigger rising edge. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | L | Audio | Left channel audio output, scaled to +/-5V and hard-clamped to that range. |
| 4 | R | Audio | Right channel audio output, scaled to +/-5V and hard-clamped to that range. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| PITCH | Knob | -2.0 to +2.0 | 0.0 | Pitch offset in V/OCT units. Added to the V/OCT input before conversion to a playback rate multiplier. At 0.0, playback is at original speed. At +2.0, playback is 4x faster (two octaves up). At -2.0, playback is 4x slower (two octaves down). |

## Details

### Signal Flow

GAReplay reads from a shared stereo buffer that is written by the GAResample module. The buffer contains audio normalized to +/-1.0 (GAResample divides its input by 5V during recording). On output, GAReplay multiplies the buffer values by 5.0 to restore the original +/-5V audio range, then hard-clamps the result to +/-5V.

### Trigger and Start Position

On each rising edge of the TRIG input (crossing above 1.0V), playback begins. The START input is sampled at the moment of the trigger to determine where in the buffer playback starts. The START CV uses a 0-10V unipolar range mapped linearly across the full buffer length: 0V = beginning, 5V = midpoint, 10V = end. Values outside 0-10V are clamped. Once playback is initiated, it proceeds forward through the buffer until the end is reached, at which point playback stops and the outputs go silent. There is no looping -- each trigger produces a single one-shot pass through the buffer from the start position to the end.

### Pitch Control

The PITCH knob and V/OCT input are summed to produce a combined V/OCT value, which is then clamped to +/-10V. This value is converted to a playback rate multiplier using a fast 2^x approximation (fastExp2). The multiplier determines how many samples of the buffer are advanced per DSP sample:

- Combined V/OCT = 0: multiplier = 1.0 (original speed)
- Combined V/OCT = +1: multiplier = 2.0 (double speed, one octave up)
- Combined V/OCT = -1: multiplier = 0.5 (half speed, one octave down)
- Combined V/OCT = +2: multiplier = 4.0 (four times speed, two octaves up)

Because the playback position advances by a fractional amount per sample, the module uses linear interpolation between adjacent buffer samples to produce smooth, artifact-reduced output at non-integer playback rates.

### Relationship with GAResample

GAResample records stereo audio into a shared buffer (up to 2 seconds at sample rate). GAReplay reads from that same buffer. The buffer length is dynamically set by GAResample based on its LENGTH parameter and recording state. If GAResample has not yet recorded anything (buffer length is 0 or the buffer pointers are null), GAReplay outputs silence regardless of trigger activity.

## Tips

- Pair with a GAResample module to create a record-and-replay sampler chain. Record a phrase with GAResample, then re-trigger it rhythmically with GAReplay.
- Use the V/OCT input with a sequencer to play the recorded buffer at different pitches, turning a recorded phrase into a melodic instrument.
- Send negative V/OCT values or set the PITCH knob below zero to slow playback down for time-stretch and granular-style effects.
- Modulate the START input with an LFO or random source to trigger playback from different positions in the buffer, creating slicing and stutter effects.
- Combine the START CV with a sequencer to pick specific slice points in the buffer, similar to a breakbeat slicer workflow.
- Since playback is one-shot (no looping), fast re-triggering at high rates can produce choppy, glitch-style textures. Use a clock divider or pattern generator to control the rhythmic density.
- At extreme positive pitch values (+2 octaves with additional V/OCT), the buffer plays back very quickly, producing short, percussive bursts even from long recordings.
- Multiple GAReplay modules can read from the same shared buffer simultaneously, each with different pitch and start settings, to create layered polyphonic textures from a single recording.
