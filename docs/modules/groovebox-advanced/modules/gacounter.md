# GACounter

A trigger-driven counter module that increments on each incoming clock pulse and outputs the current count as a raw integer value. When the count reaches a configurable maximum, it wraps back to zero and emits a trigger pulse on its wrap output. This module is useful for building sequencer step logic, clock division patterns, and cyclic event counting within a GrooveboxAdvanced patch.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | CLK | Trigger | Clock trigger input. Each rising edge (crossing above 1V) increments the internal counter by one. If the counter reaches the MAX value on this edge, it wraps to zero and fires the WRAP output. |
| 1 | RST | Trigger | Reset trigger input. A rising edge (crossing above 1V) immediately resets the counter to zero. Reset is processed before the clock edge within each sample, so if both arrive simultaneously the counter resets first, then increments to 1 on the same sample. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | CNT | Control (CV) | Count output. Outputs the current counter value as a raw integer voltage: 0V, 1V, 2V, 3V, and so on up to (MAX - 1)V. The value is not normalized to a 0-10V range -- it represents the literal step index, which makes it directly useful for driving step-selection inputs on sequencer modules. |
| 3 | WRAP | Trigger | Wrap trigger output. Emits a 10V pulse lasting 1ms each time the counter wraps from (MAX - 1) back to zero. This fires on the same clock edge that causes the wrap. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| MAX | Knob (integer) | 1 - 64 | 16 | Maximum count value. The counter increments from 0 to (MAX - 1), then wraps back to zero on the next clock edge. The knob snaps to integer values. Setting MAX to 1 means every clock pulse triggers a wrap. |

## Details

### Signal Flow

The module maintains a single integer counter (`count`) that starts at zero. On each audio sample, the DSP processes reset and clock inputs in sequence:

1. **Reset detection**: If the RST input crosses above 1V (rising edge), the counter is immediately set to zero.
2. **Clock detection**: If the CLK input crosses above 1V (rising edge), the counter increments by one. If the new count equals or exceeds MAX, the counter wraps to zero and a 1ms trigger pulse begins on the WRAP output.
3. **Output**: The CNT output is set to the current counter value cast to a float (e.g., count = 5 outputs 5.0V). The WRAP output is set to 10V during an active wrap pulse, or 0V otherwise.

### Edge Detection

Both inputs use standard VCV Rack rising-edge detection with a 1V threshold. The module stores the previous sample's value for each input and triggers only when the signal transitions from at or below 1V to above 1V. This means sustained high signals do not cause repeated triggers.

### Wrap Pulse Timing

The wrap trigger pulse is generated at a fixed 1ms duration, calculated as `sampleRate * 0.001` samples. The pulse begins on the same sample that the counter wraps and counts down to zero over subsequent samples. Only one wrap pulse can be active at a time; if the counter wraps again before the previous pulse finishes, the pulse timer restarts.

### Parameter Sync

The MAX knob value (a float stored on the UI module) is cast to an integer and synced to the DSP module each process cycle. Because the knob snaps to integer values, the cast is lossless under normal operation. The count comparison uses `>=`, so even if a non-integer value were somehow provided, the behavior would remain correct.

### Count Output Range

The CNT output is a raw integer value, not a normalized 0-10V signal. For a MAX of 16, the output ranges from 0V to 15V. This is intentional -- it allows the counter to directly drive step indices in sequencer modules or other count-sensitive inputs without requiring additional scaling. If you need a normalized signal, patch the CNT output through an attenuator or math module.

## Tips

- Use GACounter as a step sequencer index generator: connect a GAClock to the CLK input and patch CNT into a module that accepts a step-selection CV. Set MAX to your desired sequence length.
- Chain multiple counters for hierarchical counting. Connect the WRAP output of one counter to the CLK input of the next to create a divide-by-N chain -- for example, a 4-step counter feeding an 8-step counter creates a 32-step cycle.
- Connect a reset trigger to RST to synchronize the counter with the start of a pattern or song section. This is especially useful when the counter drives a sequencer and you want it to restart from step zero on a song reset.
- Set MAX to small values (2, 3, or 4) and use the WRAP output as a clock divider. A MAX of 4 turns every 4th clock pulse into a wrap trigger, effectively dividing the clock by 4.
- For polyrhythmic patterns, run two counters from the same clock with different MAX values (e.g., 3 and 4). Their WRAP outputs will fire at different rates, creating a 12-step polyrhythmic cycle.
- Since the CNT output produces raw integer voltages, you can patch it into a quantizer or use it with a sample-and-hold to generate stepped CV sequences without needing a dedicated sequencer module.
