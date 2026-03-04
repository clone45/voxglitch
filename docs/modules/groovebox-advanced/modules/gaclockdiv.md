# GAClockDiv

A clock divider module that outputs a trigger once every N input clock triggers. It takes a clock signal and divides it by an adjustable integer ratio, producing a slower, evenly-spaced trigger stream. This is useful for creating rhythmic subdivisions, half-time patterns, or slower modulation clocks derived from a master clock.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | CLK | Trigger | Clock input. Each rising edge (crossing 1.0V upward) is counted toward the division ratio. |
| 1 | RST | Trigger | Reset input. A rising edge (crossing 1.0V upward) resets the internal trigger count to zero and immediately stops any in-progress output pulse. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | OUT | Trigger | Divided clock output. Emits a 10V trigger pulse (1ms duration) every time the internal count reaches the division ratio. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| RATIO | Knob (integer snap) | 1 - 16 | 2 | Clock division ratio. The module outputs one trigger for every N incoming clock triggers. A ratio of 1 passes every trigger through; a ratio of 4 outputs one trigger for every four input triggers. |

## Details

GAClockDiv works by counting rising edges on the CLK input and emitting a trigger pulse on OUT every N edges, where N is determined by the RATIO parameter.

**Edge detection:** Both the CLK and RST inputs use rising-edge detection with a 1.0V threshold. A rising edge is registered when the signal transitions from below 1.0V to at or above 1.0V.

**Reset behavior:** When a rising edge is detected on the RST input, the internal count is immediately set to zero and any currently active output pulse is terminated. The reset is processed before the clock edge within the same sample, so if both inputs trigger simultaneously, the reset takes priority and the clock edge that follows will be counted as the first tick of a new cycle.

**Output pulse generation:** When the internal count reaches the division ratio, the module starts a 10V output pulse with a fixed duration of 1ms (determined by a `pulseWidth` of 0.001 seconds). The pulse phase advances each sample at a rate of `1.0 / (pulseWidth * sampleRate)`. While the pulse is active, the output holds at 10V. Once the phase reaches 1.0, the output returns to 0V.

**Integer snapping:** The RATIO knob snaps to integer values. The division ratio is cast to an integer and clamped to the range 1 through 16 before use.

**Counting logic:** The counter increments on each clock rising edge, starting from 0 after a reset (or at initialization). When the count reaches the division ratio, it resets to 0 and fires the output pulse. This means the first output pulse occurs N clock ticks after startup or reset.

## Tips

- Set RATIO to 2 to create a half-time clock from your master clock, useful for driving modules that should run at half speed.
- Chain multiple GAClockDiv modules in series (output of one into CLK of the next) to create compound divisions. For example, dividing by 3 and then by 4 produces a divide-by-12 clock.
- Use the RST input to synchronize the divider's phase with other modules. Sending a reset at the start of a pattern ensures the divided clock aligns with beat 1.
- With RATIO set to 1, the module acts as a trigger conditioner: it converts any signal with a rising edge above 1V into a clean 10V, 1ms trigger pulse.
- Feed the divided output into step sequencers or trigger sequencers to advance them at slower rates relative to other modules sharing the same master clock.
- Use different RATIO values on parallel GAClockDiv modules fed by the same clock to create polyrhythmic trigger patterns (e.g., one at 3 and another at 4 for a 3-against-4 polyrhythm).
