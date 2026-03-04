# GASequencer16

A 16-step CV sequencer that outputs bipolar control voltage based on slider values for each step. It advances through its steps on incoming clock triggers and supports variable sequence length via a knob and CV input. Multiple GASequencer16 modules can be daisy-chained via CHAIN IN/OUT ports to create longer composite sequences that play end-to-end.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | CLK | Trigger | Clock input. Each rising edge above 1V advances the sequencer to the next step. On the very first clock after a reset (or after the module is activated by a chain-in trigger), the sequencer stays at step 0 rather than advancing, ensuring step 0 is always heard. Clocks arriving within approximately 10ms of a reset edge are ignored to prevent accidental double-advances (standard VCV Rack reset-clock guard). |
| 1 | RST | Trigger | Reset input. A rising edge above 1V resets the step counter to 0 and re-arms the first-step guard so the next clock will play step 0. If the module is part of a chain (CHAIN IN is connected), receiving a reset also reactivates this module as the first sequencer in the chain. |
| 2 | C.IN | Trigger | Chain input. A rising edge above 1V activates this sequencer and resets it to step 0. When connected, this port marks the module as part of a chain: the module starts inactive and only plays when it receives a chain-in trigger from the previous sequencer. After completing its full sequence it deactivates and fires a pulse on CHAIN OUT. |
| 3 | LEN | Control (CV) | Length CV modulation input. The voltage offsets the LEN knob value at a scale of 1.6 steps per volt. For example, +5V adds 8 steps and -5V subtracts 8 steps. The effective length is clamped to the 1-16 range after modulation. A 0V signal (or no cable) leaves the knob value unchanged. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 4 | OUT | Control (CV) | CV output. When the sequencer is active, outputs the current step's slider value scaled to a -5V to +5V bipolar range (a slider at 0.0 outputs -5V, at 0.5 outputs 0V, and at 1.0 outputs +5V). When the sequencer is inactive (waiting for a chain-in trigger), outputs 0V. The output changes only on clock edges. |
| 5 | C.OUT | Trigger | Chain output. Fires a 1ms, 10V trigger pulse when the sequencer completes a full cycle (wraps past the last step) and the module is part of a chain. Connect this to the CHAIN IN of the next sequencer to create multi-sequencer chains. When not in a chain (no cable on CHAIN IN), this port remains at 0V even at end-of-cycle. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| LEN | Knob | 1.0 - 16.0 | 16.0 | Sequence length in steps. Controls how many of the 16 steps play before the sequence wraps back to step 0 (or passes control to the next chained sequencer). For example, setting LEN to 8 means only steps 0-7 play. The value is combined with the LEN CV input before being clamped to 1-16. |
| S1 - S16 | Slider | 0.0 - 1.0 | 0.5 | Step value sliders. Each slider sets the CV value for its corresponding step. The slider value is mapped to bipolar output: 0.0 produces -5V, 0.5 produces 0V, and 1.0 produces +5V. The currently playing step is highlighted in the UI. |

## Details

### Signal Flow

On each rising clock edge (signal crosses above 1V), the sequencer advances to the next step. The OUT port outputs the current step's slider value scaled to the -5V to +5V range using the formula `(sliderValue - 0.5) * 10.0`. This produces a standard VCV Rack bipolar CV signal where the slider's center position (0.5) corresponds to 0V.

When the sequencer is inactive (in chained mode, waiting for activation), the OUT port outputs 0V regardless of step values.

### First-Step Guard

After a reset or chain-in activation, the module sets a "first step" flag. The next clock edge clears this flag but does not advance the step counter, keeping it at step 0. This prevents a common issue in modular sequencing where a simultaneous reset and clock cause step 0 to be skipped. Clocks arriving within approximately 10ms after a reset edge are also ignored entirely (the duration is `sampleRate / 100` samples).

### Sequence Length and CV Modulation

The effective sequence length is computed as `LEN_knob + LEN_CV * 1.6`, clamped to 1-16 and truncated to an integer. This means the LEN CV input provides 1.6 steps of offset per volt. A full -5V to +5V sweep can shift the length by 16 steps total. The length determines when the step counter wraps: if `currentStep >= effectiveLength`, the counter resets to 0.

### Chaining Behavior

Chaining allows multiple GASequencer16 modules to play sequentially, creating patterns longer than 16 steps.

**Standalone mode (no cable on CHAIN IN):** The module is always active. It plays through its steps and wraps back to step 0 indefinitely. The CHAIN OUT port stays at 0V.

**Chained mode (cable connected to CHAIN IN):** The module starts inactive (producing 0V on OUT). When it receives a rising edge on CHAIN IN, it activates and resets to step 0. It then plays through its steps normally. When it reaches the end of its sequence (step counter wraps), it deactivates itself and emits a 1ms, 10V pulse on CHAIN OUT. This pulse can trigger the next sequencer in the chain.

A reset trigger reactivates the module even when chained, making it the first active sequencer in the chain. This ensures the entire chain restarts cleanly from the first module.

### Pulse Duration

The CHAIN OUT trigger pulse lasts exactly 1ms regardless of sample rate (`sampleRate * 0.001` samples). This is long enough for reliable edge detection by the downstream module.

### Output Voltage Mapping

The step sliders use a 0.0 to 1.0 internal range which is converted to bipolar voltage on output:

| Slider Position | Output Voltage |
|----------------|----------------|
| 0.0 (minimum)  | -5V            |
| 0.25           | -2.5V          |
| 0.5 (center)   | 0V             |
| 0.75           | +2.5V          |
| 1.0 (maximum)  | +5V            |

## Tips

- For melodic sequences, connect the OUT port to a VCO's V/Oct input through a GAQuantize module. The bipolar output range covers a wide pitch range, and the quantizer will snap voltages to musical scale degrees.
- Chain two GASequencer16 modules together for a 32-step sequence: connect the first module's C.OUT to the second module's C.IN, and feed the same clock to both CLK inputs. The first module plays its 16 steps, then hands off to the second.
- Use the LEN CV input with an LFO or random source to create evolving patterns that change length over time. A slow triangle LFO on LEN produces sequences that gradually lengthen and shorten, creating variation from a fixed set of step values.
- Set all sliders to 0.5 (center/0V) as a starting point, then adjust individual steps up or down for precise bipolar CV control. This is useful for fine pitch offsets or subtle modulation patterns.
- For rhythmic modulation, set the LEN knob to short values like 3, 4, or 5 to create repeating CV patterns that cycle at a different rate than a longer gate sequence, producing polymetric effects.
- Pair with a GAGateSeq of the same length to create note-on/note-off patterns alongside pitch CV. Feed both the same clock and reset signals so they stay synchronized.
- To create call-and-response melodic phrases, chain two GASequencer16 modules with different step values and set each to a length of 8. The first plays its 8-step phrase, then the second plays its 8-step response, creating a 16-step composite melody.
- Send the same reset signal to all chained modules to ensure the chain always restarts cleanly from the first sequencer.
