# GAQuantize

A voltage quantizer that snaps incoming pitch CV to the nearest note in a selectable musical scale. It supports 16 scales ranging from chromatic through common Western modes to exotic scales like Hirajoshi and Hungarian Minor. The root note is adjustable, allowing the scale to be transposed to any of the 12 semitones. An optional CV input can modulate the active scale selection in real time.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Control (CV) | Pitch CV input in VCV Rack's 1V/octave standard (0V = C4). The voltage is converted to semitones, quantized to the nearest note in the selected scale, and output as quantized pitch CV. |
| 1 | SCL | Control (CV) | Scale selection CV modulation input. The value is added directly to the SCALE parameter's index (as a whole-number offset) and the result is clamped to the valid range of 0-15. For example, if SCALE is set to "Major" (index 1) and the SCL input receives 4.0, the effective scale becomes "Blues" (index 5). |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Control (CV) | Quantized pitch CV output in 1V/octave standard. The output is guaranteed to land exactly on a note within the selected scale, preserving the correct octave of the input. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| SCALE | Dropdown | 0-15 (Chromatic, Major, Minor, Pent Maj, Pent Min, Blues, Dorian, Phrygian, Lydian, Mixolydian, Locrian, Harm Min, Mel Min, Hung Min, Hirajoshi, Whole Tone) | Chromatic (0) | Selects which musical scale the input is quantized to. The selection can be offset by the SCL CV input. When set to Chromatic every semitone is valid, so the output simply rounds to the nearest semitone. |
| ROOT | Dropdown | 0-11 (C, C#, D, D#, E, F, F#, G, G#, A, A#, B) | C (0) | Sets the root note of the scale. This shifts the entire scale pattern so that the selected note becomes the tonic. Internally, the root is subtracted from the input before scale lookup and added back afterward, so the scale intervals are always applied relative to this root. |

## Details

### Signal Flow

The quantizer is stateless -- it processes each sample independently with no internal memory or smoothing.

1. **Semitone Conversion**: The input voltage is multiplied by 12 to convert from the 1V/octave standard into semitones (e.g., 1.0V becomes 12 semitones).

2. **Root Offset Removal**: The ROOT parameter value (0-11 semitones) is subtracted from the semitone value. This reframes the input so that the root note sits at interval 0, allowing the scale lookup table to be applied generically regardless of key.

3. **Octave and Note Decomposition**: The adjusted semitone value is split into an octave number (via floor division by 12) and a note-within-octave remainder (0-11). Negative remainders are wrapped into the previous octave to ensure the note index is always non-negative.

4. **Nearest Semitone Rounding**: The fractional note-within-octave value is rounded to the nearest integer semitone. If rounding produces 12, it wraps to 0 in the next octave.

5. **Scale Lookup**: The rounded semitone is checked against the selected scale's 12-element boolean array. If the note is already in the scale, it is kept. Otherwise, the algorithm searches outward in both directions (up to 6 semitones away) and returns the first scale note found. The search checks below before above at each distance, so ties favor the lower neighbor.

6. **Voltage Reconstruction**: The quantized note index, octave, and root offset are recombined into semitones and divided by 12 to produce the output voltage in 1V/octave standard.

### Scale CV Modulation Behavior

The SCL input is added directly to the SCALE dropdown's numeric index (not scaled or divided). The sum is clamped to the 0-15 range. This means sending integer-valued CVs into SCL will cleanly switch between scales. Fractional values are truncated to integers during the cast. Because the value is clamped rather than wrapped, sending large positive CVs will saturate at Whole Tone (index 15) and large negative CVs will saturate at Chromatic (index 0).

### Available Scales

| Index | Name | Notes in Scale |
|-------|------|----------------|
| 0 | Chromatic | C C# D D# E F F# G G# A A# B (all 12) |
| 1 | Major | C D E F G A B (7 notes) |
| 2 | Minor | C D Eb F G Ab Bb (7 notes) |
| 3 | Pent Maj | C D E G A (5 notes) |
| 4 | Pent Min | C Eb F G Bb (5 notes) |
| 5 | Blues | C Eb F F# G Bb (6 notes) |
| 6 | Dorian | C D Eb F G A Bb (7 notes) |
| 7 | Phrygian | C Db Eb F G Ab Bb (7 notes) |
| 8 | Lydian | C D E F# G A B (7 notes) |
| 9 | Mixolydian | C D E F G A Bb (7 notes) |
| 10 | Locrian | C Db Eb F Gb Ab Bb (7 notes) |
| 11 | Harm Min | C D Eb F G Ab B (7 notes) |
| 12 | Mel Min | C D Eb F G A B (7 notes, ascending form) |
| 13 | Hung Min | C D Eb F# G Ab B (7 notes) |
| 14 | Hirajoshi | C D Eb G Ab (5 notes) |
| 15 | Whole Tone | C D E F# G# A# (6 notes) |

All scale patterns are relative to the ROOT parameter, so choosing ROOT = D with SCALE = Major produces D E F# G A B C#.

## Tips

- Place GAQuantize between a sequencer or pattern generator and a VCO's V/OCT input to constrain melodies to a specific key and scale. This guarantees every note is musically "in key" even when the source CV is imprecise or randomly generated.
- Use the SCL CV input with a sequencer or step-based CV source to switch scales on specific beats, creating chord-progression-like harmonic movement within a single melody line.
- Pair with a GASlew module after the quantizer output to add portamento (glide) between the quantized steps. The quantizer provides clean pitch targets and the slew smooths the transitions.
- Feed a random or noise source into IN to generate random melodies that are always in key. Combine with Pentatonic Major or Pentatonic Minor for pleasant-sounding random sequences, since pentatonic scales have no dissonant intervals.
- Use the ROOT parameter to transpose a pattern to a different key without changing the sequence data. For example, changing ROOT from C to G transposes the entire output up a perfect fifth while keeping the same scale shape.
- For microtonal or chromatic passages, set SCALE to Chromatic. The module will still round fractional voltages to the nearest semitone, which is useful for cleaning up noisy or drifting CV sources.
- The Blues scale (index 5) includes the characteristic "blue note" (the tritone between F and F#), making it effective for blues, funk, and rock-influenced generative patches.
- Chain two GAQuantize modules to create conditional scale behavior: the first quantizes to a broad scale, and the second further restricts to a subset.
