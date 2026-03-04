# GAInteger

A utility module that outputs a fixed integer value. Unlike GAConstant, which outputs a bipolar floating-point voltage, GAInteger outputs a raw unscaled integer in the range 0 to 64. This makes it well-suited for providing step counts, sequence lengths, division ratios, or other discrete numeric parameters to modules that expect integer-valued control signals.

## Inputs

None.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | OUT | Control (CV) | Outputs the raw integer value set by the VALUE parameter. The output is not normalized or voltage-scaled -- it directly outputs the integer as a float (0.0 to 64.0). |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| VALUE | Integer Input | 0 - 64 | 0 | Sets the integer value to output. Uses a picker-style control that snaps to whole numbers. The value is cast to an integer in the DSP engine before being output, ensuring clean integer values on the OUT port. |

## Details

### Signal Flow

The GAInteger module is entirely stateless with no inputs. Its processing is a single operation each sample:

1. **Parameter Sync**: The `syncToDSP` method copies the VALUE parameter from the UI module to the DSP module, casting the float-stored parameter to an integer. This truncation ensures that only whole numbers reach the DSP engine.

2. **Output**: The DSP `process()` method casts the integer value back to a float and writes it directly to the OUT port. There is no voltage scaling, normalization, or clamping applied -- the raw integer value appears on the output as-is.

### Raw Output vs. Voltage-Scaled Output

GAInteger deliberately does not scale its output to standard VCV Rack voltage ranges. Where GAConstant multiplies its value by 5.0 to produce a -5V to +5V signal, GAInteger outputs the literal integer value (e.g., a setting of 16 produces an output of 16.0). This design choice makes it appropriate for controlling modules that interpret their inputs as raw numeric values rather than as voltages, such as sequence length parameters, clock division counts, or pattern indices.

### Stateless Design

The module has no `reset()` behavior because there is no internal state to clear. The `syncFromDSP` method is a no-op since no DSP state needs to propagate back to the UI. The entire signal path is: UI parameter -> integer cast -> float output.

## Tips

- Use GAInteger to set the length or step count of a sequencer module. Patch the OUT port into a length or count input to define how many steps a pattern uses.
- Pair with a GAClockDiv or GAClockMult module to provide a fixed division or multiplication ratio via a clean integer value.
- Use GAInteger to select a specific pattern index or preset number on modules that accept numeric selectors.
- When you need a stable integer-valued signal that never drifts between whole numbers, GAInteger is preferable to using a GAConstant with manual tuning, since GAInteger guarantees integer output through its internal cast.
- Combine multiple GAInteger modules with a GAMath module to compute derived integer values (e.g., half the step count, or an offset pattern index).
