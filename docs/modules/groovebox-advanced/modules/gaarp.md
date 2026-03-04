# GAArp

An arpeggiator module that generates melodic note sequences from a chosen musical scale within a configurable octave range. It advances through the notes in response to an external clock/trigger, outputting 1V/octave CV, gate, and trigger signals. Five arpeggio modes (Up, Down, Up-Down, Down-Up, Random) provide different traversal patterns across the scale.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | STEP | Trigger | Clock/step input. A rising edge above 1V advances the arpeggiator to the next note in the sequence. The gate output follows this signal, going high while STEP is above 1V and low when it drops below 1V. |
| 1 | RST | Trigger | Reset input. A rising edge above 1V resets the arpeggiator to the beginning of the pattern. In Down-Up mode, the direction is reset to descending; in all other modes, it resets to the start. After a reset, step triggers are ignored for approximately 10ms to prevent simultaneous clock/reset glitches. |
| 2 | SCL | Control (CV) | Scale CV modulation. This input modulates the effective pattern length. The voltage is scaled by 0.3 and added to the LEN parameter as an integer offset, allowing external CV to shorten or lengthen the active portion of the arpeggio on the fly. |
| 3 | LOW | Control (CV) | Low pitch CV offset. This input is averaged with the HIGH CV input and added directly to the output CV voltage as a pitch offset. At +5V combined, the output shifts up approximately 2.5 octaves. |
| 4 | HIGH | Control (CV) | High pitch CV offset. This input is averaged with the LOW CV input and added directly to the output CV voltage as a pitch offset. Use both LOW and HIGH together or individually to transpose the arpeggio in real time. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 5 | CV | Control (CV) | 1V/octave pitch CV output. Outputs the voltage corresponding to the current note in the arpeggio, plus any pitch offset from the LOW and HIGH CV inputs. The voltage is derived from the selected scale, root note, and octave range, where each semitone equals 1/12 V. |
| 6 | GATE | Gate | Gate output. Goes high (10V) while the STEP input is above 1V and drops to 0V when STEP falls below 1V. The gate width is therefore determined by the incoming clock pulse width. |
| 7 | TRIG | Trigger | Trigger output. Emits a short 1ms pulse (10V for 48 samples at 48kHz) on each rising edge of the STEP input. This provides a consistent short trigger regardless of the incoming clock pulse width. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| SCALE | Dropdown | Chromatic, Major, Minor, Pent Maj, Pent Min, Blues, Dorian, Mixolydian, Harm Min, Whole Tone | Major | Selects the musical scale used to generate the arpeggio notes. The scale determines which of the 12 chromatic semitones are included in the note array. For example, Major includes 7 notes per octave, Pentatonic Major includes 5, and Chromatic includes all 12. |
| ROOT | Dropdown | C, C#, D, D#, E, F, F#, G, G#, A, A#, B | C | Sets the root note of the scale. The root note shifts all scale degrees by the selected number of semitones. For example, selecting D with the Major scale produces D Major (D, E, F#, G, A, B, C#). |
| OCT L | Knob | -2.0 - 2.0 | -1.0 | Low octave boundary. Sets the lowest octave offset for the arpeggio range. The note array is built from OCT L up to OCT H, so setting this to -2 extends the arpeggio two octaves below the root. Values are rounded to integers internally. If OCT L is set higher than OCT H, the values are automatically swapped. |
| OCT H | Knob | -2.0 - 2.0 | 1.0 | High octave boundary. Sets the highest octave offset for the arpeggio range. With the default range of -1 to +1, the arpeggio spans three octaves. Increasing this value adds more notes to the upper end of the arpeggio. |
| MODE | Dropdown | Up, Down, Up-Down, Down-Up, Random | Up | Selects the traversal pattern through the note array. Up ascends from the lowest note to the highest and wraps. Down descends from the highest to the lowest and wraps. Up-Down ping-pongs ascending then descending. Down-Up ping-pongs descending then ascending. Random picks a note at random from the available notes on each step. |
| LEN | Knob | 1.0 - 32.0 | 8.0 | Pattern length. Limits how many notes from the beginning of the note array are used in the arpeggio. If LEN is smaller than the total number of notes in the array, only the first LEN notes are traversed. If LEN exceeds the note count, it is clamped to the available notes. This value is also modulated by the SCL CV input. |

## Details

### Note Array Construction

When the module initializes or any of the SCALE, ROOT, OCT L, or OCT H parameters change, it rebuilds an internal note array. The construction process iterates through every octave from OCT L to OCT H (inclusive) and, within each octave, checks each of the 12 chromatic semitones against the selected scale's interval pattern. Notes that belong to the scale are added to the array as 1V/octave voltages, calculated as `(octave * 12 + semitone + root) / 12.0`. This means 0V corresponds to C at octave 0, and each semitone adds approximately 0.0833V. The maximum note array size is 128 entries.

When the note array is rebuilt, the current step position resets to 0 and the traversal direction resets to its initial state.

### Arpeggio Traversal

On each rising edge of the STEP input, the module advances to the next note according to the selected MODE:

- **Up**: Increments the step index and wraps around to 0 when the effective length is reached.
- **Down**: Decrements the step index and wraps around to the last position when it goes below 0.
- **Up-Down**: Increments until reaching the top, then reverses direction and decrements until reaching the bottom, creating a ping-pong effect. The boundary notes are played once per direction change (no double-striking at the turnaround).
- **Down-Up**: Same ping-pong behavior but starting in the descending direction.
- **Random**: Uses a deterministic linear congruential generator (LCG) to select a random index within the effective length on each step.

The effective length is the LEN parameter plus a small offset from the SCL CV input (voltage multiplied by 0.3, truncated to integer), clamped between 1 and 32, and further clamped to the actual number of notes in the array.

### CV Modulation

The LOW and HIGH CV inputs are averaged together and added as a pitch offset to the output CV. This means patching a single CV source into either LOW or HIGH will shift the output pitch by half the incoming voltage, while patching the same signal into both will shift by the full voltage. This provides a flexible transposition mechanism.

### Clock and Reset Behavior

The module follows VCV Rack trigger conventions: a rising edge is detected when the signal crosses above 1V from below. After a reset trigger is received, step triggers are ignored for approximately 10ms (sampleRate/100 samples) to prevent simultaneous clock and reset pulses from causing a one-step advance immediately after reset -- a common issue with hardware-style clock modules.

### Gate and Trigger Outputs

The GATE output directly tracks the STEP input state: it is 10V while STEP is above 1V and 0V otherwise. This means the gate width matches the incoming clock pulse width, which is useful for controlling envelope sustain on downstream oscillator or synth modules.

The TRIG output is independent of clock pulse width. It fires a fixed-duration 48-sample pulse (approximately 1ms at 48kHz) on each step advance, providing a consistent short trigger suitable for percussive envelopes or trigger-based modules.

## Tips

- Connect a clock module to the STEP input and the CV output to an oscillator's V/OCT input for an instant arpeggio. Use the GATE output to drive a VCA or envelope generator for note articulation.
- Use the LEN parameter to create shorter melodic patterns within a larger scale range. For example, set the octave range wide but LEN to 4 or 5 to create a tight repeating motif that only uses the lowest notes of the scale.
- Modulate the SCL input with a slow LFO or sequencer to dynamically change the pattern length over time, creating evolving arpeggios that shift between shorter and longer phrases.
- The Up-Down and Down-Up modes produce musical ping-pong patterns that work well for ambient or trance-style arpeggios. Because the boundary notes are not double-struck, the rhythm stays even.
- Patch a quantized CV sequence into the LOW or HIGH inputs to transpose the entire arpeggio to different keys on each bar or phrase, creating chord-progression-following arpeggios.
- Use the RST input with a clock divider to restart the arpeggio pattern at regular intervals (e.g., every 8 or 16 steps), keeping it synchronized with other sequenced elements in the patch.
- Random mode with a short LEN (3-5 notes) and a pentatonic scale produces pleasant generative melodies with no dissonant intervals.
- The TRIG output is useful for driving a separate percussive envelope or sample trigger that should fire with consistent timing regardless of the clock duty cycle, while the GATE output works better for sustained sounds where note length matters.
