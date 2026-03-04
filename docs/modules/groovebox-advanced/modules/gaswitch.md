# GASwitch

A 2-input signal switch that selects between two input signals. When the SEL control is low, input A is passed to the output. When SEL is high, input B is passed instead. Selection can be controlled either by a CV/gate signal or by a manual toggle parameter.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| SWITCH_A_IN | A | Audio | First input signal, selected when SEL is low |
| SWITCH_B_IN | B | Audio | Second input signal, selected when SEL is high |
| SWITCH_SEL_IN | SEL | Trigger | Selection control signal. Voltages >= 1V select input B; voltages < 1V select input A |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| SWITCH_OUT | OUT | Audio | The currently selected input signal, passed through without modification |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| SEL | Switch | A / B | A | Selects which input is routed to the output when no CV is connected to the SEL port |

## Details

GASwitch is a clean signal selector with no crossfading or interpolation -- it performs a hard switch between its two inputs.

**Selection logic:**

The module first checks whether the SEL input port has a meaningful signal by testing if the absolute value of the CV exceeds 0.1V. This acts as a simple "cable connected" detector:

- **CV present (|SEL| > 0.1V):** The CV signal controls selection. Voltages >= 1.0V select input B; voltages below 1.0V select input A. This follows the VCV Rack standard gate threshold of approximately 1V.
- **No CV (|SEL| <= 0.1V):** The manual SEL toggle parameter controls selection. Position A routes input A to the output; position B routes input B.

The selected input signal is passed directly to the output with no gain change, filtering, or other processing. Unconnected inputs default to 0V, so if only one input is patched the switch will alternate between that signal and silence.

## Tips

- Use a gate or trigger signal on the SEL input to rhythmically alternate between two different sound sources or modulation signals.
- Pair with a Clock or PatGen module to create structured A/B pattern switching synced to the sequencer.
- Route two different modulation sources (e.g., two LFOs with different rates) into A and B, then switch between them for evolving modulation textures.
- Chain multiple GASwitch modules to build more complex signal routing. For three or four inputs, consider using GASwitch3 or GASwitch4 instead.
- Use with an Envelope or Slew module on the output to smooth transitions and avoid clicks when switching between audio-rate signals.
- Connect a Prob (probability gate) module to the SEL input for randomized signal selection.
