# GAProb

A probability gate module that selectively passes or blocks incoming triggers based on a configurable probability threshold. Each time a trigger arrives, the module rolls a random number and only forwards the trigger to the output if the roll falls below the combined probability set by the knob and any incoming CV modulation.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Trigger input; a rising edge is detected when voltage crosses above 1V |
| 1 | PROB | Control (CV) | Probability CV input; 0-10V adds 0-100% to the base probability set by the knob |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Trigger | Trigger output; emits a 10V pulse of 1ms duration when a trigger passes the probability test |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| PROB | Knob | 0.0 - 1.0 | 0.5 | Base probability that an incoming trigger will pass through to the output (0 = never, 1 = always) |

## Details

GAProb acts as a probabilistic gate between a trigger source and a trigger destination. Its core behavior is straightforward: on each rising edge detected at the TRIG input, the module generates a random number between 0 and 1 and compares it against a total probability value. If the random number is less than the total probability, a trigger pulse is emitted at the output. Otherwise, nothing happens and the trigger is effectively swallowed.

The total probability is calculated by summing the base probability (set by the PROB knob, ranging from 0.0 to 1.0) and the CV input voltage divided by 10 (converting the standard 0-10V range into 0.0-1.0). The result is clamped to the 0.0-1.0 range, so even if the knob and CV together exceed 1.0, the probability caps at 100%. Similarly, negative CV or very low knob settings will bottom out at 0%.

Rising edge detection uses a 1V threshold, consistent with VCV Rack conventions. The module tracks the previous sample's trigger voltage, and a rising edge is registered when the current sample exceeds 1V and the previous sample was at or below 1V. This means sustained high voltages will not repeatedly fire -- only the initial transition triggers evaluation.

When a trigger passes the probability test, the output emits a clean 10V pulse with a fixed duration of 1ms (0.001 seconds). The pulse length is decremented each sample based on the current sample rate, ensuring consistent timing regardless of the engine sample rate. If a new trigger passes while a pulse is still active, the pulse timer is reset to a full 1ms, effectively extending the output.

The random number generation uses the C standard library `rand()` function, which provides a uniform distribution. Each trigger evaluation is independent -- there is no memory of previous outcomes, no pattern detection, and no attempt to distribute triggers evenly over time. At 50% probability, it is entirely possible (though unlikely) to get long runs of consecutive passes or blocks.

## Tips

- Set the PROB knob to 0.5 for a coin-flip gate that passes roughly half of all incoming triggers, creating organic, non-repeating variations of a rhythmic pattern.
- Use a GAConstant or GALFO module connected to the PROB CV input to dynamically change the probability over time. A slow LFO sweeping 0-10V will smoothly transition a pattern from silence to full density and back, creating natural builds and breakdowns.
- Place GAProb between a clock or gate sequencer and a drum trigger to create humanized, non-mechanical rhythms. Even a high probability like 0.9 will occasionally drop beats, adding subtle variation.
- Chain multiple GAProb modules in series to create compound probabilities. Two modules at 0.5 each will result in an effective 25% pass rate, with each stage independently filtering triggers.
- Use different GAProb modules with different probability settings on parallel paths from the same trigger source to create layered textures where some elements play more frequently than others. For example, a kick drum at 90% probability paired with a hi-hat at 40% creates a sparse, evolving beat.
- Connect a GACompare or GACounter output to the PROB CV input for conditional probability changes -- for example, increasing the probability of a fill trigger after a certain number of bars have elapsed.
- Set the knob to 0.0 and control probability entirely via CV for fully automated probability control from sequencers or envelopes.
