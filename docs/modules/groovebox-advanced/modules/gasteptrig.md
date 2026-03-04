# GAStepTrig

A utility module that generates a short trigger pulse, gate signal, and velocity CV output whenever the sequencer reaches a step where this patch is assigned. It allows internal patch modules (such as envelopes, sample players, and oscillators) to know when a step has been triggered, enabling retriggering behavior within a continuously-processing patch.

## Inputs

None.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TRIG | Trigger | Outputs a 10V trigger pulse lasting 1ms each time the patch is triggered on a sequencer step. |
| 1 | GATE | Gate | Outputs a 10V gate signal that stays high for the duration of the 1ms trigger pulse. |
| 2 | VEL | Control (CV) | Outputs the velocity of the most recent trigger as a 0-10V CV signal. Holds the last velocity value until the next trigger (sample-and-hold behavior). Defaults to 10V (full velocity). |

## Parameters

None.

## Details

### Signal Flow

The GAStepTrig module acts as a bridge between the GrooveboxAdvanced sequencer engine and the internal patch modules. Its signal path is:

1. **Trigger Detection**: The `GAProcessor` calls `setStepTrigger(triggered, velocity)` on all GAStepTriggerDSP instances in the patch every sample. When the sequencer advances to a step where this patch is assigned, `stepTriggered` is set to `true` for a single sample and `velocity` is updated to the step's velocity value.

2. **Trigger Pulse Generation**: When `stepTriggered` becomes `true`, the module starts a 1ms trigger pulse. During each call to `process()`, the remaining pulse time is decremented by one sample period (`1.0 / sampleRate`). While the pulse is active, both the TRIG and GATE outputs are held at 10V. Once the pulse expires, both outputs return to 0V.

3. **Velocity Output**: The VEL output produces `velocity * 10.0V`, where velocity is a 0.0 to 1.0 value set by the sequencer. The velocity value is held from the most recent trigger until the next one arrives (sample-and-hold), so downstream modules can read it at any time during the step.

### Legacy Gate Mode

The module also supports edge detection on an external gate signal (`externalGate`) for backward compatibility with the older voice-allocation mode. When `externalGate` transitions from below 0.5V to above 0.5V, a new trigger pulse is started. This path is used by `GAVoice` in non-continuous processing mode via `setStepTriggerGate()`.

### Continuous Processing Context

In GrooveboxAdvanced's continuous processing mode, all patches run continuously every sample regardless of which step is active. The GAStepTrig module provides the mechanism for patches to distinguish between "running in the background" and "actively triggered on this step." Without this module, patches would have no way to retrigger envelopes, restart samples, or respond to step changes.

### Safety Behavior

If the sample rate is zero or negative (which can occur before audio processing is fully initialized), all three outputs are forced to 0V to prevent undefined behavior.

## Tips

- Patch the TRIG output into envelope trigger inputs or sample player trigger inputs to retrigger sounds on each sequencer step where the patch is assigned.
- Use the GATE output when a downstream module requires a gate rather than a trigger (though both signals have the same 1ms duration in this module).
- Route the VEL output into a VCA or envelope depth input to create velocity-sensitive patches where louder steps produce stronger sounds.
- Place a GAStepTrig module in any patch that needs to respond to step changes, such as patches containing envelopes, triggered sample players, or any module that should reset or retrigger on each step.
- Combine the VEL output with a Scale module to remap the 0-10V velocity range to a narrower or offset range suitable for specific parameter modulation.
- Multiple GAStepTrig modules can be placed in the same patch; all of them will fire simultaneously when the step is triggered, which can be useful if different parts of a complex patch need independent trigger routing.
