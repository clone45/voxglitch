# GAPatArp

A pattern-based arpeggiator module that reads from a library of 128 preset arpeggio patterns stored in an external JSON file (`res/data/arp_patterns.json`). Rather than generating arpeggios algorithmically from a scale and mode, GAPatArp steps through pre-authored semitone sequences, outputting 1V/octave CV. It features configurable pattern length, a clock divider for rhythmic variation, and an ADD input for pitch transposition and chaining multiple instances together.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | STEP | Trigger | Clock/step input. A rising edge above 1V advances the arpeggiator to the next step in the selected pattern. The clock divider (DIV parameter) determines how many rising edges are required before the step actually advances. |
| 1 | RST | Trigger | Reset input. A rising edge above 1V resets the step position and clock divider counter to zero. After a reset, step triggers are ignored for approximately 1ms (sampleRate/1000 samples) to prevent simultaneous clock/reset glitches from causing an immediate one-step advance. |
| 2 | ADD | Control (CV) | Additive pitch CV input. This voltage is added directly to the pattern's CV output after the semitone-to-voltage conversion. At +1V the output shifts up one octave; at +0.0833V it shifts up one semitone. This input is designed for chaining multiple GAPatArp instances or for external transposition. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | CV | Control (CV) | 1V/octave pitch CV output. Outputs the semitone value at the current step position within the selected pattern, divided by 12 to convert to V/OCT, plus the voltage present at the ADD input. A semitone value of 12 in the pattern data produces +1V (one octave up); a value of -12 produces -1V (one octave down). |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| PAT | Dropdown | 0 - 127 (128 named patterns) | 0 | Selects which arpeggio pattern to use. The pattern names and data are loaded from `res/data/arp_patterns.json` at startup. If the JSON file is missing or corrupt, all 128 patterns default to silence (semitone offset 0 at every step). |
| LEN | Knob (integer) | 1 - 32 | 8 | Pattern length in steps. The step counter wraps back to 0 when it reaches this value, so only the first LEN steps of the 32-step pattern are used. Reducing LEN creates shorter, tighter melodic loops from the beginning of the selected pattern. |
| DIV | Knob (integer) | 1 - 16 | 1 | Clock divider. The module counts incoming STEP triggers and only advances the pattern step when the count reaches the DIV value. At DIV=1 every trigger advances the step. At DIV=4, only every fourth trigger advances the pattern, effectively slowing the arpeggio to one-quarter of the clock rate. The divider counter resets to zero each time a step advance occurs and also on reset. |

## Details

### Pattern Data

GAPatArp reads from a shared singleton pattern store (`PEPatArpData::PatternStore`) that holds 128 patterns, each consisting of 32 steps. Each step contains a semitone offset (int8, range -24 to +24) and a gate flag (boolean). The CV output uses the semitone values; the gate data is present in the store but is not currently used by GAPatArp's DSP.

The pattern data is loaded from `res/data/arp_patterns.json` at startup. The JSON format expects an array of pattern objects, each with a `name` string, a `cv` array of 32 integer semitone offsets, and a `gate` array of 32 integer values (0 or 1). Power users can edit this JSON file to customize or create new arpeggio patterns.

### Signal Flow

1. **Reset detection**: On each sample, the module checks for a rising edge on the RST input (signal crossing above 1V from at or below 1V). When detected, the step position and divider counter are both set to 0, and a clock-ignore window of approximately 1ms is activated.

2. **Step detection**: If the clock-ignore window has expired, the module checks for a rising edge on the STEP input. When detected, the divider counter increments. If the counter reaches the DIV value, it resets to 0 and the step position advances by one, wrapping at the LEN value.

3. **CV output**: The current step's semitone value is read from the pattern store, divided by 12.0 to convert to V/OCT, and then the ADD input voltage is summed in. The result is written to the CV output.

All parameter values (pattern index, length, divider) are clamped to their valid ranges on every sample to prevent out-of-bounds access.

### Clock-Ignore Window

After a reset trigger, step triggers are ignored for `sampleRate / 1000` samples (approximately 1ms at any sample rate). This prevents a common issue where clock and reset signals arrive on the same sample or within a few samples of each other, which would cause the pattern to immediately advance one step past the reset position.

### V/OCT Conversion

The conversion from pattern data to output voltage is straightforward: `cvOut = semitones / 12.0 + addInput`. Since the 1V/octave standard defines 1V per 12 semitones, dividing the semitone offset by 12 produces the correct voltage. A pattern step with a value of 7 (a perfect fifth) outputs approximately 0.583V, a value of 12 (one octave) outputs 1.0V, and a value of -12 outputs -1.0V.

## Tips

- Connect a clock module to the STEP input and the CV output to an oscillator's V/OCT input for instant preset arpeggios. Pair with an envelope generator triggered by the same clock for note articulation.
- Use the DIV parameter to create slower-moving arpeggios relative to the master clock. Setting DIV to 2 or 4 while other modules run at the base clock rate creates a layered rhythmic feel with the arpeggio moving at half or quarter speed.
- Chain multiple GAPatArp instances by connecting the CV output of one into the ADD input of the next. The second instance adds its own pattern's voltage on top of the first, creating compound arpeggios that combine two patterns simultaneously.
- Use a constant voltage source or a quantized pitch CV into the ADD input to transpose the arpeggio to different keys. Since ADD is summed directly with the V/OCT output, +1/12 V shifts up one semitone, and +1V shifts up one full octave.
- Experiment with short LEN values (2-4) to create tight ostinato figures from just the opening steps of a longer pattern. Automate LEN with an external step sequencer to gradually reveal more of the pattern over time.
- Send a clock divider output into the RST input to restart the arpeggio at regular intervals (e.g., every 8 or 16 beats), keeping it locked to phrase boundaries in the arrangement.
- Combine DIV with LEN for unusual rhythmic relationships. For example, LEN=3 with DIV=2 creates a 3-note pattern that advances every other clock pulse, producing a 6-clock cycle before repeating.
- Edit the `res/data/arp_patterns.json` file to create custom arpeggio patterns tailored to a specific composition. Each pattern supports semitone offsets from -24 to +24, spanning a four-octave range centered on the root.
