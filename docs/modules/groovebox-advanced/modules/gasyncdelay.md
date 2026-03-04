# GASyncDelay

A stereo clock-synced delay effect that automatically locks its delay time to musical subdivisions of an incoming clock signal. Rather than setting delay time manually, the module measures the period between incoming clock pulses and derives the delay time as a selectable musical division (from 1/64 note to whole note). This makes it straightforward to create tempo-locked echo effects that stay in sync with the rest of the patch.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | L | Audio | Left channel audio input to be delayed and processed. |
| 1 | R | Audio | Right channel audio input to be delayed and processed. |
| 2 | CLK | Trigger | Clock input used to measure tempo. The module detects rising edges (crossing upward through 1.0V) and measures the time between consecutive edges to determine the clock period. Valid periods range from 0.1 seconds to 4.0 seconds (approximately 15-600 BPM). Without a clock signal connected, the delay defaults to a 0.5-second clock period (equivalent to 120 BPM). |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | L | Audio | Left channel audio output. The signal is a crossfade between the dry input and the delayed signal, controlled by the MIX parameter. Output is clamped to +/-10V. |
| 4 | R | Audio | Right channel audio output. The signal is a crossfade between the dry input and the delayed signal, controlled by the MIX parameter. Output is clamped to +/-10V. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| FDBK | Knob | 0.0 - 0.95 | 0.4 | Feedback amount. Controls how much of the delayed signal is fed back into the delay buffer along with the incoming audio. Higher values produce more echo repeats with slower decay. At 0.0, only a single echo is produced. The maximum value of 0.95 prevents runaway self-oscillation while still allowing long echo trails. |
| MIX | Knob | 0.0 - 1.0 | 0.5 | Dry/wet mix. At 0.0 only the dry input is heard; at 1.0 only the delayed (wet) signal is heard. At 0.5 (default) the dry and wet signals are mixed equally. The crossfade is linear: output = input * (1 - MIX) + delayed * MIX. |
| DIV | Knob (snapping) | 0 - 12 | 8 (1/4 note) | Musical division selector. The knob snaps to integer values, each corresponding to a musical subdivision of the clock period. The available divisions are: 0 = 1/64 note (0.0625x), 1 = 1/32 note (0.125x), 2 = 1/32 triplet (0.1667x), 3 = 1/16 note (0.25x), 4 = 1/16 triplet (0.333x), 5 = 1/8 note (0.5x), 6 = dotted 1/8 (0.75x), 7 = 1/4 triplet (0.667x), 8 = 1/4 note (1.0x), 9 = dotted 1/4 (1.5x), 10 = 1/2 note (2.0x), 11 = dotted 1/2 (3.0x), 12 = whole note (4.0x). The multiplier is applied to the measured clock period to determine the actual delay time. |

## Details

### Signal Flow

1. **Clock Sync**: On each sample, the module accumulates elapsed time. When a rising edge is detected on the CLK input (signal crosses upward through 1.0V), the module measures the elapsed time since the previous rising edge. If this period falls within the valid range of 0.1 to 4.0 seconds (corresponding roughly to 15-600 BPM), it is stored as the current clock period. This provides a continuously updating tempo measurement. Until two clock edges have been received, the module uses a default period of 0.5 seconds (120 BPM).

2. **Delay Time Calculation**: The delay time is computed by multiplying the measured clock period by the selected division ratio from the DIV knob. For example, with a clock period of 0.5 seconds (120 BPM) and DIV set to 8 (1/4 note, ratio 1.0x), the delay time is 0.5 seconds. With DIV set to 5 (1/8 note, ratio 0.5x), the delay time would be 0.25 seconds. The delay time in samples is clamped to a maximum of 262,143 samples (approximately 5.4 seconds at 48 kHz).

3. **Interpolated Delay Read**: Delayed samples are read from stereo circular buffers using linear interpolation between two adjacent samples. This ensures smooth, artifact-free output when the delay time is not an exact integer number of samples and when the clock period changes between pulses.

4. **Feedback Path**: The value written into each delay buffer at each sample is `input + delayed * feedback`. The delayed signal is attenuated by the feedback amount on each pass through the buffer. With feedback at 0.0, only a single echo is produced. With feedback approaching 0.95, each echo retains 95% of the previous echo's amplitude, producing a long, slowly decaying trail of repeats.

5. **Dry/Wet Mix**: The output is a linear crossfade between the dry input and the wet delayed signal for each stereo channel, computed as `output = input * (1 - mix) + delayed * mix`.

6. **Output Clamping**: The final stereo output is clamped to the +/-10V range to prevent excessively loud signals from high feedback settings or hot inputs.

### Buffer Behavior

The delay uses a stereo circular buffer pair with a maximum capacity of 262,144 samples (approximately 5.4 seconds at 48 kHz). The effective buffer length is dynamically adjusted to be slightly larger than the current delay time in samples (with a 1,000-sample margin). The write position wraps around the buffer length, overwriting the oldest samples. Calling reset clears both buffers to silence and resets all clock timing state to defaults.

### Clock Period Tracking

The module performs simple period measurement rather than averaging. Each valid clock pulse pair updates the period immediately. This means the delay time responds quickly to tempo changes, but may also shift slightly if the incoming clock has jitter. For stable operation, use a clean clock source.

## Tips

- Connect the same clock signal that drives your sequencer to the CLK input. This ensures the delay echoes land exactly on musical beat divisions, creating rhythmically coherent repeats.
- For a classic dub delay, set DIV to 5 (1/8 note) or 6 (dotted 1/8), FDBK to 0.4-0.6, and MIX around 0.3-0.4. The echoes will bounce in time with the beat, filling space between notes.
- Use dotted note divisions (6 = dotted 1/8, 9 = dotted 1/4) to create the "ping-pong" rhythmic feel common in pop and electronic music, where echoes fall between straight beat divisions.
- Triplet divisions (2 = 1/32 triplet, 4 = 1/16 triplet, 7 = 1/4 triplet) add a swing or shuffle feel to the echoes, which works well with jazz, hip-hop, or any music with a triplet groove.
- For ambient washes, set DIV to 10 or higher (1/2 note or longer), FDBK to 0.7-0.9, and MIX to 0.4-0.5. The long delay times with high feedback create slowly building, evolving textures.
- Since the module is stereo, you can feed different signals into the L and R inputs or process a stereo source directly. Both channels share the same delay time, feedback, and mix settings.
- Set MIX to 1.0 (fully wet) when using GASyncDelay in a parallel effects chain with a GAMixer, so you can control the dry/wet balance at the mixer level instead.
- Use high feedback values (0.85-0.95) with caution. While the output is clamped to +/-10V and feedback is capped below 1.0, high feedback creates dense echo trails that can build up significantly before decaying. This can be a creative tool for sound design, but may overwhelm a mix if left unchecked.
- Very short divisions (1/64, 1/32) at moderate to fast tempos produce comb-filter-like effects rather than distinct echoes, which can add metallic or resonant coloring to the sound.
- If no clock is connected, the module defaults to 120 BPM timing. You can use it this way as a fixed-time delay where the DIV knob selects among musically-related delay times relative to 120 BPM.
