# GASequencer

An 8-step CV sequencer that outputs a voltage for each step in sequence, advancing on incoming clock triggers. Multiple GASequencer modules can be daisy-chained via their CHAIN IN and CHAIN OUT ports to create longer sequences (16, 24, 32 steps, etc.), where each sequencer plays its 8 steps then passes control to the next in the chain.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | CLK | Trigger | Clock input. A rising edge above 1V advances the sequencer to the next step. On the first clock after a reset (or after the module becomes active via chain), the sequencer stays on step 0 rather than advancing, ensuring step 0 is not skipped. Clock triggers are ignored for approximately 10ms (sampleRate/100 samples) after a reset to prevent simultaneous clock/reset glitches. Clock triggers are only processed when the sequencer is in the active state. |
| 1 | RST | Trigger | Reset input. A rising edge above 1V resets the step counter to 0 and re-arms the first-step protection flag. After a reset, a brief clock-ignore window (~10ms) prevents an immediate advance. If the sequencer is part of a chain (CHAIN IN is connected), a reset also reactivates this sequencer, making it the starting point of the chain. |
| 2 | C.IN | Trigger | Chain input. A rising edge above 1V activates this sequencer and resets its step counter to 0, beginning playback from step 1 on the next clock. When a trigger is received on this port, the module marks itself as part of a chain. While unchained, the sequencer is always active. While chained, the sequencer is dormant until it receives a chain trigger or a reset signal. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | OUT | Control (CV) | CV output. When the sequencer is active, outputs the current step's value scaled to a bipolar range of -5V to +5V. The internal step values range from 0.0 to 1.0, and the output mapping is `(stepValue - 0.5) * 10.0`, so a slider at 0.0 outputs -5V, at 0.5 outputs 0V, and at 1.0 outputs +5V. When the sequencer is inactive (dormant in a chain, waiting for its turn), the output is 0V. |
| 4 | C.OUT | Trigger | Chain output. Emits a 1ms pulse at 10V when the sequencer completes its 8-step cycle and is part of a chain. This pulse triggers the next sequencer's CHAIN IN port to begin its sequence. The pulse is only emitted when the sequencer is chained (has received a trigger on CHAIN IN at some point). When not chained, the sequencer loops its 8 steps indefinitely and never fires CHAIN OUT. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| S1 | Slider | 0.0 - 1.0 | 0.5 | CV value for step 1. Maps to the output range -5V to +5V via the formula `(value - 0.5) * 10`. The default of 0.5 produces 0V output. |
| S2 | Slider | 0.0 - 1.0 | 0.5 | CV value for step 2. Same mapping as S1. |
| S3 | Slider | 0.0 - 1.0 | 0.5 | CV value for step 3. Same mapping as S1. |
| S4 | Slider | 0.0 - 1.0 | 0.5 | CV value for step 4. Same mapping as S1. |
| S5 | Slider | 0.0 - 1.0 | 0.5 | CV value for step 5. Same mapping as S1. |
| S6 | Slider | 0.0 - 1.0 | 0.5 | CV value for step 6. Same mapping as S1. |
| S7 | Slider | 0.0 - 1.0 | 0.5 | CV value for step 7. Same mapping as S1. |
| S8 | Slider | 0.0 - 1.0 | 0.5 | CV value for step 8. Same mapping as S1. |

## Details

### Signal Flow

On each audio sample, the DSP module checks three inputs for rising edges (threshold: 1V). The processing order is: chain input, reset input, then clock input. The chain input and reset input both reset the step counter to 0. The clock input advances the step counter, but only when the sequencer is active.

The output is computed every sample as `(stepValues[currentStep] - 0.5) * 10.0` when active, or 0V when inactive. This means the output responds immediately to slider changes without waiting for the next clock edge.

### Step Advancement and First-Step Protection

When the sequencer resets (via RST or C.IN), a `firstStep` flag is set. On the next rising clock edge, instead of advancing from step 0 to step 1, the module clears the flag and stays on step 0. This ensures that step 0 always plays for a full clock period after reset, rather than being skipped by a clock that arrives simultaneously or shortly after the reset.

After a reset, clock triggers are ignored for approximately 10ms (calculated as `sampleRate / 100` samples). This clock-ignore window works in conjunction with the first-step flag to handle simultaneous clock and reset signals cleanly.

### Chaining Behavior

The chaining system allows multiple GASequencer modules to act as a single longer sequence. The chain state machine works as follows:

1. **Unchained mode** (default): When CHAIN IN has never received a trigger, `chainInConnected` is false. The sequencer is always active, loops its 8 steps indefinitely, and never emits a CHAIN OUT pulse.

2. **Chained mode**: Once CHAIN IN receives a trigger, `chainInConnected` is set to true permanently (for the current session). The sequencer becomes active and begins playing from step 0.

3. **End of cycle**: When a chained sequencer's step counter wraps from step 7 back to step 0, it sets itself to inactive (output goes to 0V) and emits a 1ms trigger pulse on CHAIN OUT. This pulse activates the next sequencer in the chain.

4. **Reset in chain**: When RST fires on a chained sequencer, it reactivates itself and resets to step 0. All sequencers in a chain should receive the same reset signal so the entire chain restarts from the first sequencer.

The CHAIN OUT pulse duration is exactly 1ms regardless of sample rate, calculated as `sampleRate * 0.001` samples. The pulse outputs 10V for its duration.

### Active Step Highlighting

The UI module tracks the current active step via `syncFromDSP()`, which copies the DSP module's `currentStep` value back to the UI. The parameter configurator uses `setSliderHighlight()` to visually indicate which slider corresponds to the currently playing step.

## Tips

- For a basic 8-step melody, connect a Clock module's output to CLK, set the 8 sliders to different positions, and route OUT to a VCO's V/Oct input through a Quantize module to snap the voltages to musical scale degrees.
- To create a 16-step sequence, place two GASequencer modules and connect the first module's C.OUT to the second module's C.IN. Feed the same CLK and RST signals to both modules. The first sequencer plays its 8 steps, then the second takes over for steps 9-16 before the cycle repeats.
- Chain three or four sequencers for 24 or 32-step patterns. Each module in the chain needs the same clock and reset connections, but only the first module in the chain should NOT have anything connected to its C.IN port.
- Use the sequencer to modulate parameters other than pitch. Route OUT to a filter cutoff, a VCA level, or an effect parameter for rhythmic timbral changes synchronized to the clock.
- Since the output is bipolar (-5V to +5V), the center position of each slider produces 0V. Push sliders above center for positive voltages and below center for negative voltages. For unipolar applications (0V to 10V), route the output through a Scale module to shift and scale the range.
- When a chained sequencer is inactive, its output is 0V. If you need the last step's value to hold rather than drop to zero, route the output through a sample-and-hold or slew limiter to maintain the voltage between active periods.
- Combine with a GateSeq module to pair CV values with gate patterns. Use the same clock and reset signals for both modules so their steps stay synchronized.
