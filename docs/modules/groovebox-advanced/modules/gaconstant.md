# GAConstant

A simple utility module that outputs a fixed constant voltage. It provides a user-adjustable bipolar CV signal with no inputs required, making it useful for supplying static voltages to modulation inputs, offsets, or fixed parameters without needing an external CV source.

## Inputs

None.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | OUT | Control (CV) | Outputs a constant voltage derived from the VALUE knob. The internal value (-1.0 to +1.0) is scaled by 5.0, producing an output range of -5V to +5V following VCV Rack's bipolar CV standard. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| VALUE | Knob | -1.0 - 1.0 | 0.0 | Sets the constant output voltage. The knob value is multiplied by 5.0 to produce the output voltage. At -1.0 the output is -5V, at 0.0 the output is 0V, and at +1.0 the output is +5V. |

## Details

### Signal Flow

The GAConstant module is entirely stateless with no inputs. Its processing consists of a single operation each sample:

1. **Voltage Scaling**: The VALUE parameter (ranging from -1.0 to +1.0) is multiplied by 5.0 to produce the output voltage. This maps the normalized internal range to VCV Rack's standard bipolar CV range of -5V to +5V.

2. **Output**: The scaled value is written to the OUT port every sample. Since there are no inputs and no internal state, the output remains perfectly stable at the set voltage until the VALUE knob is adjusted.

### Stateless Design

The module has no `reset()` behavior because there is no internal state to clear. The `syncToDSP` method copies the VALUE parameter from the UI module to the DSP module each cycle, and the DSP `process()` method simply outputs the scaled value. There is no `syncFromDSP` path since no DSP state needs to propagate back to the UI.

## Tips

- Use GAConstant to provide a fixed modulation amount to any module's CV input. For example, patch it into a filter's cutoff CV input to set a static offset on the cutoff frequency.
- Pair with a GAAtten module to create an adjustable voltage source with finer control: patch the GAConstant output into GAAtten's input and use the GAAtten AMT knob for precision scaling.
- Use two GAConstant modules set to different values and patch them into a mixer to create a fixed voltage sum, useful for biasing signals to specific operating points.
- Set the VALUE knob to +1.0 (outputting +5V) or -1.0 (outputting -5V) to provide a fixed gate-like signal for testing or holding modules in a particular state.
- When building patches that need a known reference voltage (for example, a fixed pitch via V/Oct), GAConstant provides a clean, stable source without noise or drift.
