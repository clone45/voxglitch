# GAClockMult

Clock multiplier module that takes an incoming clock signal and outputs a faster clock at an integer multiple of the original rate. Useful for generating subdivisions of a master clock to drive faster sequencer steps, hi-hats, arpeggios, or any element that needs to run at a multiple of the main tempo.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | CLK | Trigger | Incoming clock signal. Rising edges (crossing 1.0V from below) are detected and used to measure the clock period. |
| 1 | RST | Trigger | Reset signal. A rising edge clears all scheduled trigger events and stops any in-progress output pulse, resynchronizing the module. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | OUT | Trigger | Multiplied clock output. Produces 10V trigger pulses at the multiplied rate. Each pulse is 1ms wide. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| RATIO | Integer Knob | 1 - 16 | 2 | Clock multiplication factor. At 1 the output matches the input clock. At 2 the output runs twice as fast, at 4 four times as fast, and so on up to 16x. |

## Details

GAClockMult works by measuring the time interval between consecutive rising edges on the CLK input. When a new clock edge arrives, the module divides the most recently measured period by the RATIO value to determine a subdivision interval. It then pre-schedules (RATIO - 1) additional trigger times into an internal queue. The first trigger fires immediately on the incoming clock edge; the remaining triggers fire at evenly spaced intervals throughout the clock period.

**Signal flow:**

1. On every sample, an internal timer accumulates elapsed time using the host sample rate.
2. When a rising edge is detected on CLK, the module calculates the time elapsed since the previous clock edge. If this elapsed time is positive and less than the 4-second timeout, it becomes the new measured period.
3. The module immediately starts a 1ms output pulse (the first subdivision) and clears any previously scheduled triggers.
4. If the ratio is greater than 1, the module computes `period / ratio` and schedules the remaining subdivision triggers at future times.
5. On each subsequent sample, the module checks if any scheduled trigger time has been reached. If so, it pops that trigger from the queue and starts a new 1ms pulse.
6. The output is 10V while a pulse is active and 0V otherwise.

**Timeout behavior:** If no clock edge arrives within approximately 4 seconds, the measured period is considered stale and no new subdivisions are scheduled until a valid clock interval is measured again. This prevents runaway triggers when the clock source is disconnected.

**Reset behavior:** A rising edge on the RST input immediately clears all scheduled triggers and cancels any in-progress pulse. The module then waits for the next CLK edge to begin generating output again.

**Startup behavior:** The module requires at least two incoming clock edges before it can produce multiplied output, since it needs to measure the period between two edges. On the very first clock edge after initialization (or after a reset), only a single trigger is output with no subdivisions.

## Tips

- Set the RATIO to 2 or 4 to generate eighth-note or sixteenth-note subdivisions from a quarter-note master clock.
- Chain multiple GAClockMult modules to create compound multiplications (e.g., 4x into 3x for 12x overall), which can be useful for creating polyrhythmic trigger patterns.
- Use the RST input connected to the same reset signal as your sequencers to keep the multiplied clock tightly synchronized after transport resets.
- A ratio of 1 passes the clock through with re-triggered 1ms pulses, which can be useful for normalizing inconsistent pulse widths from other clock sources.
- Pair with a pattern generator or step trigger module to create fast rhythmic fills, rolls, or hi-hat patterns that stay locked to the master tempo.
