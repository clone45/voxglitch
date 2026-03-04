# GAPitch

A time-domain pitch shifter that uses variable-rate circular buffer reading to raise or lower the pitch of an audio signal. The module writes incoming audio into a fixed-size delay buffer at a constant rate while reading back from the buffer at a different rate, producing pitch-shifted output. At moderate settings it provides usable pitch transposition; at extreme settings it introduces audible artifacts characteristic of this simple delay-based approach.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | Audio input signal to be pitch shifted. The signal is internally scaled from VCV Rack's +/-5V range to a +/-1.0 normalized range before being written into the circular buffer. |
| 1 | PTCH | Control (CV) | Pitch shift CV modulation. A bipolar +/-5V signal that adds to the SHIFT knob value. The CV is divided by 5.0 and then multiplied by 0.5, so +5V adds 0.5 to the effective pitch and -5V subtracts 0.5. The combined value is clamped to the -1.0 to +1.0 range before affecting the read rate. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | Mono audio output. The pitch-shifted signal, linearly interpolated from the circular buffer and scaled back to VCV Rack's +/-5V range. Output is hard-clamped to +/-5V. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| SHIFT | Knob | -24.0 - 24.0 | 0.0 | Pitch shift amount. Despite the knob range spanning -24 to 24, the effective pitch value is clamped internally to -1.0 to 1.0 after combining with CV modulation. This means the full useful range of the knob is only -1.0 to 1.0; values beyond that range have no additional effect. At 0.0, the signal passes through at original pitch. At 1.0, the read rate doubles (one octave up). At -0.5, the read rate halves (one octave down). At -1.0, the read rate reaches zero and the output freezes. |

## Details

### Signal Flow

1. **Input Scaling**: The audio signal from the IN port is divided by 5.0, converting VCV Rack's +/-5V audio standard to a +/-1.0 internal range. The pitch CV from the PTCH port is similarly divided by 5.0 to produce a +/-1.0 modulation value.

2. **Effective Pitch Calculation**: The effective pitch shift is computed as `pitchShift + (pitchCV * 0.5)`, where `pitchShift` is the SHIFT knob value and `pitchCV` is the scaled CV input. The result is clamped to the range -1.0 to 1.0. Because the knob range (-24 to 24) far exceeds this clamp, the usable portion of the knob travel is very narrow around the center position.

3. **Buffer Write**: Each sample is written into a 65536-sample circular buffer at the current write position. The write position advances by exactly 1 sample per process call, wrapping around at the buffer boundary. At 48kHz, the buffer holds approximately 1.37 seconds of audio.

4. **Buffer Read with Interpolation**: The read position is advanced by a variable amount each sample: `readRate = 1.0 + effectivePitch`. The output sample is computed using linear interpolation between the two buffer samples surrounding the fractional read position. This produces continuous output even when the read position falls between integer sample indices.

5. **Read Rate Behavior**:
   - `effectivePitch = 0.0`: Read rate is 1.0 (original pitch, no shift).
   - `effectivePitch = 1.0`: Read rate is 2.0 (double speed, one octave up).
   - `effectivePitch = -0.5`: Read rate is 0.5 (half speed, one octave down).
   - `effectivePitch = -1.0`: Read rate is 0.0 (frozen, output holds the last read value).
   - Values between -1.0 and 0.0 produce pitch-down effects; values between 0.0 and 1.0 produce pitch-up effects.

6. **Output Scaling**: The interpolated output is multiplied by 5.0 to return to VCV Rack's +/-5V audio range and then clamped to +/-5V. A safety check sets the output to 0.0 if the value is non-finite (NaN or infinity), and the entire buffer is cleared if the read or write positions become non-finite.

### Pitch Shifting Characteristics

This module uses the simplest possible pitch-shifting algorithm: writing at a fixed rate and reading at a variable rate from a circular buffer. Unlike more sophisticated approaches (such as granular overlap-add or phase vocoder methods), this technique does not attempt to maintain phase coherence or correct for the read pointer drifting away from the write pointer over time.

When the read rate differs from 1.0, the read pointer gradually moves away from the write pointer. Eventually the read pointer will "lap" the write pointer (when pitch-shifting up) or be lapped by it (when pitch-shifting down), causing discontinuities as the reader crosses into stale or freshly overwritten buffer regions. These crossover events produce periodic clicks or glitches in the output, with the frequency of the glitches increasing as the pitch shift amount increases.

### CV Modulation Behavior

The PTCH CV input is bipolar and additive. The CV voltage is divided by 5.0 and then halved before being added to the knob value. This means:
- At +5V, the CV adds 0.5 to the effective pitch (shifting the read rate up by 0.5).
- At -5V, the CV subtracts 0.5 from the effective pitch (shifting the read rate down by 0.5).
- At 0V, the CV has no effect.

Because the final effective pitch is clamped to -1.0 to 1.0, the CV cannot push the pitch beyond the range of frozen (read rate 0.0) to one octave up (read rate 2.0), regardless of the knob setting.

## Tips

- For subtle pitch detuning effects (chorus-like), keep the SHIFT knob very close to zero (e.g., 0.01 to 0.05). The slight difference in read rate produces a slowly drifting pitch offset that, when mixed with the dry signal using a GAMixer, creates a beating/chorus effect.
- Patch an LFO into the PTCH CV input for vibrato. A slow triangle or sine LFO at low amplitude will modulate the pitch up and down rhythmically, producing a natural vibrato effect.
- Use the freeze behavior at `effectivePitch = -1.0` creatively: set the SHIFT knob to -1.0 to capture and hold the current buffer content as a sustained drone or texture.
- For pitch-shifted delay effects, place GAPitch after a GADelay module. The delay feeds a repeated signal into the pitch shifter, producing pitch-shifted echoes.
- Be aware that this module does not preserve the length of the signal. Pitch-shifting up compresses the signal in time (reads through the buffer faster), while pitch-shifting down stretches it. This is the expected behavior of a simple resampling pitch shifter rather than a time-preserving one.
- To reduce the audible artifacts at larger pitch shift amounts, follow GAPitch with a GAFilter set to low-pass mode. Filtering removes some of the high-frequency discontinuities introduced by the buffer crossover events.
- For harmonizer effects, split your audio signal and send one copy through GAPitch with SHIFT set to a musical interval (e.g., 0.26 for roughly a major third up, 0.5 for an octave down), then mix the original and shifted signals together in a GAMixer.
