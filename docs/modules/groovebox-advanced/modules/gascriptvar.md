# GAScriptVar

A utility module that outputs the value of one of the 16 global script variables managed by the GrooveboxAdvanced script engine. The script engine's SET_VAR instruction writes float values into a bank of 16 variable slots; GAScriptVar reads from a selected slot and presents that value as a control output. This allows scripted automation sequences to drive any parameter in a patch -- filter cutoff, wavetable index, volume, panning, or any other control input that accepts a CV signal.

## Inputs

None.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | OUT | Control (CV) | Outputs the raw float value stored in the selected script variable slot. The value is unscaled -- it is the exact float written by the script engine's SET_VAR instruction, with no voltage normalization or clamping applied. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| SLOT | Integer Input | 1 - 16 | 1 | Selects which of the 16 global script variable slots to read from. Displayed as 1-indexed (1 through 16) in the UI, but converted to 0-indexed internally when synced to the DSP engine. |

## Details

### Signal Flow

GAScriptVar acts as a bridge between the script engine and the patch's signal graph. The data flows through three stages:

1. **Script Engine writes to variable storage**: When the script engine executes a SET_VAR instruction, it writes a float value into one of the 16 slots in the global `scriptVars[16]` array on the main GrooveboxAdvanced module. The SET_VAR instruction specifies a slot index (0-15) and a float value. The script engine validates the slot index is within bounds before writing.

2. **Processor distributes values to DSP modules**: Before each sample is processed, the GrooveboxAdvanced `process()` method calls `setScriptVarValues()` on every voice's GAProcessor. This method iterates over all GAScriptVarDSP instances in the processor's module list and copies the value from `scriptVars[slot]` into each DSP module's `currentValue` field, where `slot` is the 0-indexed slot that module is configured to read. This happens for both per-track voice processors and the audio output voice processor.

3. **DSP module outputs the value**: The `process()` method on GAScriptVarDSP is a single operation: it writes `currentValue` to the OUT port. There is no smoothing, slew limiting, or interpolation applied -- the output reflects the exact value from the script variable array at each sample.

### Parameter Sync

The UI module stores `slotValue` as a 1-indexed float (1.0 through 16.0) for display purposes. During `syncToDSP()`, this is converted to a 0-indexed integer via `(int)slotValue - 1`, which becomes the `slot` field on the DSP module. This ensures the DSP reads from the correct position in the `scriptVars` array.

### Reset Behavior

When `reset()` is called on the DSP module, `currentValue` is set to 0.0. This means the OUT port outputs 0.0 until the next SET_VAR instruction writes a new value to the selected slot.

### Unscaled Output

Like GAInteger, GAScriptVar does not normalize its output to standard VCV Rack voltage ranges. The output is the literal float value stored by SET_VAR. Script authors control what values they write -- this could be 0.0 to 1.0 for normalized parameters, 0.0 to 10.0 for voltage-range signals, or any other range. Downstream modules or an attenuverter can be used to scale the value as needed.

## Tips

- Use the script engine's SET_VAR instruction to write automation values, then place a GAScriptVar module in your patch to route that value into any control input. For example, SET_VAR slot 1 to a value between 0.0 and 1.0, then patch a GAScriptVar (SLOT=1) into a filter cutoff input for scripted filter sweeps.
- Multiple GAScriptVar modules can read from the same slot. This lets a single SET_VAR instruction drive several parameters simultaneously -- useful for coordinated changes like adjusting both filter cutoff and resonance from one script variable.
- Combine with a GAMath module to scale or offset the raw script variable value before it reaches its destination. For instance, if SET_VAR writes values in the 0.0 to 1.0 range but a target input expects 0.0 to 5.0, use GAMath to multiply by 5.
- Use different slots for independent automation lanes. With 16 slots available, you can script complex multi-parameter automation sequences where each parameter is driven by its own variable slot.
- Since the output updates only when the script engine executes a SET_VAR instruction, the value holds steady between script steps. This makes GAScriptVar suitable for stepped or quantized automation rather than continuous modulation. For smooth transitions, consider patching the output through a slew limiter or envelope follower.
