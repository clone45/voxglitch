# GAStepRnd

A triggered random CV generator with sample-and-hold behavior. Each time a trigger is received, the module generates a new random voltage within a configurable range and holds that value at the output until the next trigger arrives.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Trigger input; a rising edge is detected when voltage crosses above 1V |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 1 | OUT | Control (CV) | Random CV output; holds the most recently generated random value between triggers |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| LOW | Knob | -10.0 to 10.0 | 0.0 | Low end of the random voltage range |
| HIGH | Knob | -10.0 to 10.0 | 1.0 | High end of the random voltage range |

## Details

GAStepRnd is a triggered sample-and-hold random voltage source. It sits idle, holding its current output value, until a rising edge is detected at the TRIG input. On each rising edge, the module generates a uniformly distributed random voltage within the range defined by the LOW and HIGH knobs, and immediately sets that value at the output. The output then remains at that voltage until the next trigger arrives.

Rising edge detection uses a 1V threshold, consistent with VCV Rack conventions. The module tracks the previous sample's trigger voltage, and a rising edge is registered when the current sample exceeds 1V and the previous sample was at or below 1V. Sustained high voltages will not repeatedly fire -- only the initial transition triggers a new random value.

The random range is defined by two knobs: LOW sets the lower bound and HIGH sets the upper bound. Both range from -10V to +10V, covering the full VCV Rack voltage range. If LOW is set higher than HIGH, the module automatically swaps the two values before generating the random number, so the output will always fall within the expected range regardless of knob positions. This means there is no need to worry about which knob is set higher -- the module handles either configuration gracefully.

The random number generation uses the C standard library `rand()` function, producing a normalized value between 0 and 1, which is then scaled and offset to fit within the [low, high] range using linear interpolation: `output = low + random * (high - low)`. The distribution is uniform, meaning all values within the range are equally likely to be generated. Each trigger evaluation is independent with no memory of previous outputs.

On module reset, the held output value returns to 0V and the trigger detection state is cleared. When the module is first instantiated with default parameters, the output range is 0V to 1V, producing small positive control voltages suitable for subtle modulation.

## Tips

- Use GAStepRnd with a clock trigger to create stepped random melodies. Set LOW to 0.0 and HIGH to 10.0, then route the output through a GAQuantize module to snap the random voltages to musical scale degrees.
- Pair two GAStepRnd modules with different ranges to independently randomize pitch and velocity (or amplitude) for a drum voice, creating organic, evolving percussion patterns.
- Set a narrow range (for example, LOW = 4.5 and HIGH = 5.5) to generate subtle random variations around a center value. This works well for adding humanization to filter cutoff, panning, or other parameters that benefit from small fluctuations.
- Drive the TRIG input with a GAProb module to combine probability-based triggering with random value generation. This creates sparse, unpredictable modulation that only changes on some beats.
- Use a GAClockDiv module to trigger GAStepRnd at a slower rate than the main clock, creating random values that change every 2, 4, or 8 steps rather than every beat. This produces a more musical, phrase-level randomness.
- Set LOW and HIGH to the same value to effectively create a constant voltage source that only updates on trigger -- useful for debugging or as a baseline before dialing in a range.
- Use negative ranges (for example, LOW = -5.0 and HIGH = 0.0) to generate random attenuation or bipolar modulation signals. A range of -5.0 to 5.0 provides a full bipolar random CV source.
