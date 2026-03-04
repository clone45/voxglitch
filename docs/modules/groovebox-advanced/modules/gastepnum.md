# GAStepNum

A utility module that outputs the current step number of the host GrooveboxAdvanced sequencer. The output reflects the active sequencer position and updates automatically as the sequencer advances, providing a real-time readout of which step (0 to 63) is currently playing. This makes it useful for step-dependent modulation, conditional logic, and positional effects within a patch.

## Inputs

None.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | OUT | Control (CV) | Outputs the current sequencer step number as a raw float value. The range is 0.0 to 63.0, corresponding to the 64 possible steps in the GrooveboxAdvanced sequencer. The value is not voltage-scaled -- it is the literal step index. |

## Parameters

None.

## Details

### Signal Flow

The GAStepNum module is entirely passive from a DSP perspective. It has no inputs and no user-configurable parameters. Its value is set externally by the host sequencer via the GAProcessor, which writes the current step number into every GAStepNumDSP instance before processing each sample.

1. **Step Injection**: During each processing cycle, the GAProcessor calls `setCurrentStep(float step)`, which iterates through all registered GAStepNumDSP modules and sets their `currentStep` field to the host's current step value. This value originates from the GrooveboxAdvanced module's internal sequencer position (the `currentStep` variable in the main module).

2. **Output**: The DSP `process()` method writes `currentStep` directly to the OUT port. No clamping, scaling, or transformation is applied.

### Step Range and Sequencer Behavior

The GrooveboxAdvanced sequencer supports up to 64 steps (defined by `PATCH_SEQ_STEPS = 64`). The actual number of active steps depends on the user-configured `sequenceLength` parameter (1 to 64). The step number output will always fall within the range 0 to `sequenceLength - 1`, since the sequencer wraps at the configured length.

The step value reflects the sequencer's current position regardless of sequencer mode. Whether the sequencer is running in forward, backward, pendulum, or random mode, the OUT port always outputs the actual step index that is currently active. Steps with a "Skip" state are bypassed during sequencing, so the output will jump over skipped positions.

### Stateless Design

The module has no internal state to reset. The `reset()` method is a no-op. The `syncToDSP` and `syncFromDSP` methods are also no-ops, since there are no parameters to synchronize between the UI and DSP layers. The step value is injected directly by the processor, bypassing the normal parameter sync mechanism.

### Raw Output

Like GAInteger, GAStepNum outputs raw numeric values rather than voltage-scaled signals. A step index of 12 produces an output of 12.0, not a voltage-mapped value. Downstream modules that expect standard VCV Rack voltage ranges (e.g., 0-10V or -5V to +5V) should use a GAScale module to map the step range into the desired voltage range.

## Tips

- Patch the OUT port into a GACompare module to create step-dependent behavior. For example, compare the step number against a threshold to enable an effect only on later steps in the sequence.
- Use with GAMath to derive modulation from step position. For instance, divide the step number by the sequence length (via a GAInteger) to get a normalized 0-to-1 ramp that progresses across the sequence.
- Feed the step number into a GAScale module to create a voltage ramp synchronized to the sequencer, useful for filter sweeps or amplitude envelopes that span the full sequence length.
- Combine with a GALogic or GAProb module for step-conditional triggering, such as applying probability gates only on even-numbered steps (use GAMath to compute step modulo 2, then compare).
- Use multiple GAStepNum modules in different patches -- each will receive the same host step value, which can be useful for coordinating step-aware behavior across tracks.
