# GASwitch4

A 4-input CV switch that selects one of four input signals based on a SEL control voltage or a manual parameter. The SEL value is interpreted as an integer index (0 through 3) to choose between inputs A, B, C, and D. Designed to work naturally with ScriptVar integer values.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| SWITCH4_A_IN | A | Audio | First input signal, selected when SEL index is 0 |
| SWITCH4_B_IN | B | Audio | Second input signal, selected when SEL index is 1 |
| SWITCH4_C_IN | C | Audio | Third input signal, selected when SEL index is 2 |
| SWITCH4_D_IN | D | Audio | Fourth input signal, selected when SEL index is 3 |
| SWITCH4_SEL_IN | SEL | Control (CV) | Selection control voltage. The value is floored to an integer and clamped to the range 0-3 to determine which input is routed to the output |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| SWITCH4_OUT | OUT | Audio | The currently selected input signal, passed through without modification |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| SEL | Integer Input | 0 - 3 | 0 | Selects which input (A=0, B=1, C=2, D=3) is routed to the output when no CV is connected to the SEL port |

## Details

GASwitch4 is a clean 4-way signal selector that performs a hard switch between its four inputs with no crossfading or interpolation.

**Selection logic:**

The module first checks whether the SEL input port has a meaningful signal by testing if the absolute value of the CV exceeds approximately 0.001V. This acts as a "cable connected" detector:

- **CV present (|SEL| > 0.001V):** The CV voltage controls selection. The voltage is floored to an integer and clamped to the range [0, 3]:
  - SEL < 1.0V selects input A
  - 1.0V <= SEL < 2.0V selects input B
  - 2.0V <= SEL < 3.0V selects input C
  - SEL >= 3.0V selects input D
- **No CV (|SEL| <= 0.001V):** The manual SEL integer parameter controls selection. Values 0 through 3 map directly to inputs A through D.

The selected input signal is passed directly to the output with no gain change, filtering, or other processing. Unconnected inputs default to 0V, so if only some inputs are patched the switch will output silence when selecting an unpatched input.

**Note on floor behavior:** Because the selection uses `floor()`, negative CV values will be clamped to 0 (selecting input A). Fractional values between integers stay on the lower channel -- for example, a SEL voltage of 1.7V selects input B, not C.

## Tips

- Connect a ScriptVar module to the SEL input to switch between sound sources or modulation paths based on scripted integer values. Since ScriptVar outputs integers in the range the switch expects, they pair together directly with no scaling needed.
- Use a Counter module driving the SEL input to cycle through four different signals in sequence, creating round-robin or rotating patterns.
- Route four different drum sounds or samples into inputs A-D and use a sequencer or pattern generator on SEL to build variation across steps.
- Combine with a Sequencer or Sequencer16 module on the SEL input to create structured arrangements that switch between four distinct timbres or modulation sources.
- For fewer inputs, consider using GASwitch (2-input) or GASwitch3 (3-input) to save panel space.
- Use with a Slew module on the output to smooth transitions when switching between audio-rate signals, avoiding clicks at switch boundaries.
- Connect a StepRnd module to the SEL input for randomized selection among four choices, useful for generative patches.
