# GASlew

A slew rate limiter that smooths signal transitions by constraining how fast a signal can rise or fall. This is commonly used for portamento/glide effects on pitch CV, smoothing stepped control signals, or adding lag to abrupt voltage changes.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| SLEW_IN | IN | Audio | The input signal to be slew-limited. Accepts any voltage range. |
| SLEW_RATE_IN | RATE | Control (CV) | Bipolar CV modulation of both rise and fall times. Expects +/-5V (standard VCV Rack CV range). Positive voltage increases slew times; negative voltage decreases them. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| SLEW_OUT | OUT | Audio | The slew-limited output signal. Matches the input signal's voltage range once the slew has settled. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| RISE | Knob (exponential) | 0.0 - 1.0 s | 0.1 s | Controls the maximum rate at which the output can rise toward the input. Higher values produce slower, more gradual rising transitions. At 0.0, rising transitions pass through instantly. |
| FALL | Knob (exponential) | 0.0 - 1.0 s | 0.1 s | Controls the maximum rate at which the output can fall toward the input. Higher values produce slower, more gradual falling transitions. At 0.0, falling transitions pass through instantly. |

## Details

GASlew implements a classic slew rate limiter with independent rise and fall time controls. The algorithm works as follows:

1. **Initialization**: On the first sample processed, the output is set to match the input immediately. This prevents a slow ramp from 0V up to the actual signal level when the module is first activated.

2. **Bypass**: When both RISE and FALL are set to exactly 0.0 (and no CV is applied), the module bypasses all processing and passes the input directly to the output with no computational overhead.

3. **CV Modulation**: The RATE input modulates both rise and fall times simultaneously. The incoming CV is scaled from the +/-5V VCV standard range to a +/-1.0 normalized range, then added directly to the RISE and FALL parameter values. This means a +5V CV signal adds 1.0 second to both times, while a -5V signal subtracts 1.0 second. If the effective time for both rise and fall drops to zero or below after CV modulation, the module bypasses processing.

4. **Slew Calculation**: For each sample, the module computes the maximum allowed change per sample as `sampleTime / effectiveTime`. It then compares the difference between the input and the current output:
   - If the input is higher (rising), the output increases by at most the rise rate per sample.
   - If the input is lower (falling), the output decreases by at most the fall rate per sample.
   - If the difference is smaller than the allowed rate, the output snaps to the input (the slew has caught up).

5. **Exponential Scaling**: Both knobs use exponential scaling, which provides finer control at lower slew times (where small changes are more perceptible) and coarser control at higher values.

## Tips

- To create a portamento/glide effect, patch a pitch CV signal through the IN port and connect OUT to an oscillator's V/Oct input. Adjust RISE and FALL to taste for legato transitions between notes.
- Set RISE and FALL to different values for asymmetric behavior. For example, a short RISE with a long FALL creates a signal that jumps up quickly but glides down slowly, useful for creating descending pitch sweeps on gate releases.
- Use the RATE CV input with an LFO or envelope to dynamically change the slew amount over time. This can create evolving textures where transitions become more or less smooth rhythmically.
- At very high RISE and FALL values (close to 1.0 second), the module acts as a strong low-pass lag processor, heavily smoothing any input signal. This can turn a square wave LFO into a rounded, nearly sinusoidal shape.
- Setting both RISE and FALL to 0.0 effectively bypasses the module, which is useful when you want to disable glide without re-patching.
- When applied to gate signals, asymmetric slew (short RISE, longer FALL) can create simple AR-style envelope shapes.
