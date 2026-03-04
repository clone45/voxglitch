# GAGateSeq

A 32-step gate sequencer that outputs 10V or 0V based on a grid of binary on/off toggles. It advances through its steps on incoming clock triggers and supports variable sequence length. Multiple GAGateSeq modules can be daisy-chained via CHAIN IN/OUT ports to create longer composite sequences that play end-to-end.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | CLK | Trigger | Clock input. Each rising edge above 1V advances the sequencer to the next step. On the very first clock after a reset (or after the module is activated by a chain-in trigger), the sequencer stays at step 0 rather than advancing, ensuring step 0 is always heard. Clocks arriving within approximately 10ms of a reset edge are ignored to prevent accidental double-advances (standard VCV Rack reset-clock guard). |
| 1 | RST | Trigger | Reset input. A rising edge above 1V resets the step counter to 0 and re-arms the first-step guard so the next clock will play step 0. If the module is part of a chain (CHAIN IN is connected), receiving a reset also reactivates this module as the first sequencer in the chain. |
| 2 | C.IN | Trigger | Chain input. A rising edge above 1V activates this sequencer and resets it to step 0. When connected, this port marks the module as part of a chain: the module starts inactive and only plays when it receives a chain-in trigger from the previous sequencer. After completing its full sequence it deactivates and fires a pulse on CHAIN OUT. |
| 3 | LEN | Control (CV) | Length CV modulation input. The voltage offsets the LEN knob value at a scale of 3.2 steps per volt. For example, +5V adds 16 steps and -5V subtracts 16 steps. The effective length is clamped to the 1-32 range after modulation. A 0V signal (or no cable) leaves the knob value unchanged. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 4 | OUT | Gate | Gate output. Outputs 10V when the sequencer is active and the current step's toggle is ON. Outputs 0V when the step is OFF or when the sequencer is inactive (waiting for a chain-in trigger). The output follows the clock -- it changes state only on clock edges, not continuously. |
| 5 | C.OUT | Trigger | Chain output. Fires a 1ms, 10V trigger pulse when the sequencer completes a full cycle (wraps past the last step) and the module is part of a chain. Connect this to the CHAIN IN of the next GAGateSeq to create multi-sequencer chains. When not in a chain (no cable on CHAIN IN), this port remains at 0V even at end-of-cycle. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| LEN | Knob | 1.0 - 32.0 | 32.0 | Sequence length in steps. Controls how many of the 32 steps play before the sequence wraps back to step 0 (or passes control to the next chained sequencer). For example, setting LEN to 8 means only steps 0-7 play. The value is combined with the LEN CV input before being clamped to 1-32. |
| Gate Grid | 4x8 Toggle Grid | ON / OFF | All OFF | A 32-button grid (4 columns by 8 rows) representing steps 0-31. Each button toggles between ON and OFF. When ON, that step outputs 10V on the OUT port (provided the sequencer is active). The currently playing step is highlighted in the UI. |

## Details

### Signal Flow

On each rising clock edge, the sequencer advances to the next step. The OUT port is set to 10V if the current step's toggle is ON and the sequencer is active, or 0V otherwise. This means the output behaves as a gate signal that is high for the entire duration a step is active (from one clock edge to the next), not as a brief trigger pulse.

### First-Step Guard

After a reset or chain-in activation, the module sets a "first step" flag. The next clock edge clears this flag but does not advance the step counter, keeping it at step 0. This prevents a common issue in modular sequencing where a simultaneous reset and clock cause step 0 to be skipped. Clocks arriving within approximately 10ms after a reset edge are also ignored entirely (the duration is `sampleRate / 100` samples).

### Sequence Length and CV Modulation

The effective sequence length is computed as `LEN_knob + LEN_CV * 3.2`, clamped to 1-32 and truncated to an integer. This means the LEN CV input provides roughly 3.2 steps of offset per volt. A full -5V to +5V sweep can shift the length by 32 steps total. The length determines when the step counter wraps: if `currentStep >= effectiveLength`, the counter resets to 0.

### Chaining Behavior

Chaining allows multiple GAGateSeq modules to play sequentially, creating patterns longer than 32 steps.

**Standalone mode (no cable on CHAIN IN):** The module is always active. It plays through its steps and wraps back to step 0 indefinitely. The CHAIN OUT port stays at 0V.

**Chained mode (cable connected to CHAIN IN):** The module starts inactive (producing 0V on OUT). When it receives a rising edge on CHAIN IN, it activates and resets to step 0. It then plays through its steps normally. When it reaches the end of its sequence (step counter wraps), it deactivates itself and emits a 1ms, 10V pulse on CHAIN OUT. This pulse can trigger the next sequencer in the chain.

A reset trigger reactivates the module even when chained, making it the first active sequencer in the chain. This ensures the entire chain restarts cleanly from the first module.

### Pulse Duration

The CHAIN OUT trigger pulse lasts exactly 1ms regardless of sample rate (`sampleRate * 0.001` samples). This is long enough for reliable edge detection by the downstream module.

## Tips

- For basic rhythmic patterns, connect a GAClock to CLK and toggle steps ON to create kick, snare, or hi-hat patterns. Set LEN to 16 for standard 4/4 patterns or to odd values like 7 or 11 for polymetric rhythms.
- Chain two or more GAGateSeq modules together to build patterns longer than 32 steps: connect the first module's C.OUT to the second module's C.IN, and feed the same clock to both CLK inputs. The first module plays its steps, then hands off to the second.
- Use the LEN CV input with an LFO or random source to create evolving patterns that change length over time. A slow triangle LFO on LEN produces patterns that gradually lengthen and shorten.
- Combine GAGateSeq with a GAProb module on the output to add probability-based gate dropping to an otherwise fixed pattern.
- Feed the gate output into an envelope generator's gate input. Because the output stays high for the full step duration (not just a brief trigger), the envelope's sustain phase will track the clock rate, giving longer notes at slower tempos.
- To create call-and-response patterns, chain two GAGateSeq modules with different step patterns and set each to a length of 8. The first module plays its 8-step phrase, then the second plays its 8-step response, creating a 16-step composite pattern.
- Send the same reset signal to all chained modules to ensure the chain always restarts cleanly from the first sequencer.
