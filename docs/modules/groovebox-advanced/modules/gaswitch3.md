# GASwitch3

A 3-input signal switch that selects one of three inputs (A, B, or C) and routes it to a single output. The selection is controlled either by a CV signal on the SEL input or by a manual integer parameter. This module is designed to work naturally with ScriptVar integer values and other discrete control signals for routing audio or control signals through different processing paths.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | A | Audio | First input signal. Selected when the selection index equals 0. |
| 1 | B | Audio | Second input signal. Selected when the selection index equals 1. |
| 2 | C | Audio | Third input signal. Selected when the selection index equals 2. |
| 3 | SEL | Control (CV) | Selection control input. The voltage is floored to an integer and clamped to the range [0, 2] to determine which input is routed to the output. Values below 1 select A, values from 1 up to (but not including) 2 select B, and values of 2 or above select C. When this input is near zero (below 0.001 in absolute value), the manual SEL parameter is used instead. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 4 | OUT | Audio | Outputs the signal from whichever input (A, B, or C) is currently selected by the SEL control. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| SEL | Integer Input | 0 - 2 | 0 | Manual selection of which input to route to the output. 0 selects input A, 1 selects input B, and 2 selects input C. This parameter is only used when the SEL input port receives a voltage near zero (absolute value less than 0.001). When a non-zero CV is present on the SEL input, the CV takes priority over this parameter. |

## Details

### Signal Flow

GASwitch3 performs a simple multiplexer operation: it reads from one of three input ports and writes that value directly to the output port. There is no crossfading, smoothing, or interpolation applied during selection changes -- the switch is instantaneous, meaning it selects the new input's value on the very next sample.

### Selection Logic

The selection index is determined in one of two ways:

1. **CV control (SEL input)**: If the SEL input port has a voltage whose absolute value exceeds 0.001, the voltage is floored to an integer using `std::floor()` and clamped to the range [0, 2]. This means:
   - Any voltage below 1.0 (including negative values, which clamp to 0) selects input A
   - Voltages from 1.0 up to (but not including) 2.0 select input B
   - Voltages of 2.0 or above (clamped to 2) select input C

2. **Manual parameter**: If the SEL input voltage is near zero (absolute value at or below 0.001), the module falls back to using the SEL integer parameter set through the panel UI. This is also cast to an integer and clamped to [0, 2].

### Unconnected Inputs

If an input that is selected (A, B, or C) is not connected to any cable, the module outputs 0.0 for that selection, since unconnected ports return their default value of zero.

### Reset Behavior

The `reset()` method on the DSP module is a no-op. The module has no internal state beyond the parameter value -- it simply reads the current input values and selection index on every sample.

## Tips

- Pair with a GAScriptVar module to create scripted signal routing. Use the script engine's SET_VAR instruction to write 0, 1, or 2 into a variable slot, then connect a GAScriptVar reading that slot to the SEL input of GASwitch3. This lets your script dynamically swap between three different signal sources on a per-step basis.
- Use GASwitch3 to select between three different oscillator waveforms, filter configurations, or modulation sources at different points in a sequence. For example, route three differently-tuned oscillators into A, B, and C, then switch between them rhythmically using a pattern generator or sequencer.
- Chain GASwitch3 with GASwitch4 for more complex routing trees. For instance, use a GASwitch3 to pick one of three groups, and within each group use another switch to pick sub-variations, giving you up to 9 or 12 distinct signal paths.
- Since the switch is instantaneous with no crossfading, switching between audio-rate signals mid-cycle can produce clicks. If smooth transitions are needed, place a slew limiter (GASlew) after the output to soften the transition, or switch only at zero crossings by timing the SEL changes to gate or trigger events.
- Connect a GACounter or GASequencer to the SEL input to cycle through the three inputs in a predictable pattern, creating rotating signal routing effects like round-robin voice cycling or rotating delay taps.
