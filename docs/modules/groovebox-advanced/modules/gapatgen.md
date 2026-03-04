# GAPatGen

A pattern generator module that outputs deterministic CV, gate, and trigger sequences from a bank of 128 pre-computed patterns. Each pattern contains up to 32 steps of pseudo-random CV values and gate states generated from a hash-based algorithm, providing repeatable yet varied melodic and rhythmic material. A Sample-and-Hold mode filters the output so that CV only updates on gated steps, suppressing repeated notes.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | STEP | Trigger | Clock/step input. A rising edge above 1V advances the internal step counter to the next position in the pattern, wrapping around at the effective sequence length. Step triggers are ignored for approximately 10ms after a reset to prevent simultaneous clock/reset glitches. |
| 1 | RST | Trigger | Reset input. A rising edge above 1V resets the step counter to 0. After a reset, incoming step triggers are ignored for approximately 10ms (sampleRate/100 samples) to avoid a one-step advance immediately after reset. |
| 2 | PAT | Control (CV) | Pattern selection CV modulation. The voltage is scaled by 12.7 and added to the PAT parameter as an integer offset. At 10V, this adds 127 to the base pattern index, allowing full traversal of all 128 patterns via CV. The effective pattern is clamped to the 0-127 range. |
| 3 | LEN | Control (CV) | Length CV modulation. The voltage is scaled by 3.2 and added to the LEN parameter as an integer offset. At 10V, this adds 32 steps to the base length. The effective length is clamped to the 1-32 range. |
| 4 | LOW | Control (CV) | Range low CV modulation. The voltage is added directly to the LOW knob value. The effective low boundary is clamped to -10V to +10V. This shifts the lower end of the CV output range. |
| 5 | HIGH | Control (CV) | Range high CV modulation. The voltage is added directly to the HIGH knob value. The effective high boundary is clamped to -10V to +10V. This shifts the upper end of the CV output range. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 6 | CV | Control (CV) | Pattern CV output. Outputs a voltage derived from the pre-computed pattern data for the current step, rescaled from the internal -5V to +5V range into the effective LOW-to-HIGH range. In Sample-and-Hold mode, the CV only updates on steps where the gate is high, holding the previous value on gated-off steps. |
| 7 | GATE | Gate | Gate output. Outputs 10V when the current step's pre-computed gate state is high, and 0V when low. Gate density varies by pattern, ranging from approximately 30% for low-numbered patterns to approximately 93% for high-numbered patterns. In Sample-and-Hold mode, the gate is suppressed on steps where the note has not changed from the previous gated step. |
| 8 | TRIG | Trigger | Trigger output. Emits a short 1ms pulse (10V for 48 samples at 48kHz) when the step advances. In Sample-and-Hold mode, the trigger is suppressed when the note has not changed, so only steps that produce a new CV value generate a trigger. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| PAT | Dropdown | Pat 001 - Pat 128 (0-127) | Pat 001 (0) | Selects which of the 128 pre-computed patterns to use. Each pattern produces a unique deterministic sequence of CV values and gate states. Lower-numbered patterns have sparser gate density (around 30%), while higher-numbered patterns are denser (up to 93%). |
| LEN | Knob (integer) | 1 - 16 | 8 | Sets the sequence length in steps. The step counter wraps around when it reaches this value. This parameter can be extended up to 32 steps via the LEN CV input. Each pattern contains 32 steps of data, so longer lengths reveal more of the pattern. |
| LOW | Knob | -5.0 - 5.0 | -5.0 | Sets the low boundary of the CV output range. The raw pattern data (internally -5V to +5V) is normalized to a 0-1 range and then mapped into the LOW-to-HIGH range. Setting LOW higher than HIGH inverts the pattern's CV contour. |
| HIGH | Knob | -5.0 - 5.0 | 5.0 | Sets the high boundary of the CV output range. Together with LOW, this defines the voltage range of the CV output. Narrowing the range compresses the pattern's CV variation; widening it expands it. |
| S&H | Switch | OFF / ON | OFF | Enables Sample-and-Hold mode. When ON, the CV output only updates on steps where the gate is high, holding the last gated value on steps where the gate is low. Additionally, the gate and trigger outputs are suppressed on steps where the CV has not changed from the previously held value, preventing retriggering of the same note. |

## Details

### Pattern Generation

The 128 patterns are generated deterministically at first use from a hash-based pseudo-random algorithm and cached in a global table. Each pattern contains 32 CV values and 32 gate states. The CV values are computed using a multiplicative hash function (based on the Knuth multiplier 2654435761 and related constants) that takes the pattern number and step index as seeds, producing values in the -5V to +5V range. The gate states use a separate hash path with different constants, producing a boolean on/off state for each step.

Gate density is not uniform across patterns. It is calculated as `0.3 + (patternIndex % 64) / 100`, giving a range of 30% to 93%. This means lower-numbered patterns tend to have sparser rhythms with more rests, while higher-numbered patterns are denser with fewer gaps. Because the pattern index is taken modulo 64, the density repeats: patterns 0-63 and 64-127 have the same density distribution but different gate arrangements.

### CV Rescaling

The raw CV values from the pattern cache range from -5V to +5V. Before output, each value is normalized to a 0-1 range using the formula `(cvRaw + 5) / 10`, then mapped into the effective output range: `effectiveLow + normalized * (effectiveHigh - effectiveLow)`. This means:

- With default settings (LOW = -5, HIGH = +5), the output matches the raw pattern data.
- Narrowing LOW and HIGH (e.g., 0 to 2) compresses all CV variation into a smaller voltage range.
- Setting LOW higher than HIGH inverts the mapping, flipping the pattern's melodic contour.

### Sample-and-Hold Mode

When S&H is enabled, the module behaves differently in three ways:

1. **CV Hold**: The CV output only updates when the current step's gate is high. On steps where the gate is low, the output holds the last value that was sampled during a gated step.

2. **Note Change Detection**: When a step advances with the gate high, the module compares the new CV value to the previously held CV value. If the difference is less than 0.001V, the note is considered unchanged.

3. **Gate/Trigger Suppression**: If the note has not changed (same CV as the last gated step), the gate and trigger outputs are suppressed (held at 0V) even though the pattern's gate state is high. This prevents downstream envelopes from retriggering on repeated notes, producing a legato-like effect where held notes sustain through identical consecutive values.

The first gated step after a reset always triggers, since there is no previous value to compare against.

### Clock and Reset Behavior

The module follows VCV Rack trigger conventions with a 1V rising-edge threshold. After a reset trigger, step triggers are ignored for approximately 10ms (sampleRate/100 samples). This clock-ignore window prevents the common scenario where a clock and reset arrive simultaneously, which would otherwise cause the sequence to advance one step past the reset point.

### CV Modulation Scaling

Each CV input uses a different scaling factor tuned to its parameter's range:

- **PAT CV**: Multiplied by 12.7, so 10V spans the full 0-127 pattern range.
- **LEN CV**: Multiplied by 3.2, so 10V adds 32 steps (the maximum pattern length).
- **LOW/HIGH CV**: Added directly (1:1 scaling), so 1V of CV shifts the range boundary by 1V.

All CV modulation is additive to the knob value, and the result is clamped to valid bounds.

## Tips

- Use the PAT CV input with a slow LFO or step sequencer to sweep through different patterns over time, creating evolving sequences that shift between sparse and dense rhythmic variations.
- Narrow the LOW and HIGH range to a small interval (e.g., 0V to 0.5V) and feed the CV output into a quantizer for tightly constrained melodic patterns. Widen the range for more dramatic pitch variation.
- Enable S&H mode when driving a monophonic voice to create legato phrasing. The gate suppression on repeated notes means the downstream envelope will sustain through identical pitches rather than retriggering, producing smoother melodic lines.
- Modulate the LEN parameter with a clock divider's output to alternate between short and long pattern lengths on different bars, creating A/B phrase structures from a single pattern.
- Use the TRIG output to drive percussive envelopes and the GATE output to drive sustained sounds. In S&H mode, the TRIG output becomes especially useful because it only fires when the note actually changes, providing accent triggers on melodic transitions.
- Patch the GATE output into a VCA controlling another sound source to create rhythmic gating effects, using the pattern generator purely as a rhythm source while ignoring the CV output.
- Set LOW higher than HIGH to invert the pattern's melodic contour. This effectively mirrors the sequence, turning ascending phrases into descending ones.
- Combine two GAPatGen modules with different patterns and lengths to create polymetric sequences. Use one for pitch CV and the other for rhythm/gating to decouple melodic and rhythmic patterns.
