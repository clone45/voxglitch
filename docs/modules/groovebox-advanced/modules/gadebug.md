# GADebug

A utility module that displays the voltage value of a signal passing through it. GADebug acts as a transparent pass-through probe: it reads the voltage at its input, stores it for display on the module's panel, and passes the signal unchanged to its output. This makes it useful for inspecting signal values at any point in a patch without altering the signal path.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | Accepts any signal in VCV Rack's standard voltage range. The value at this input is captured for display and passed directly to the output with no modification. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 1 | OUT | Audio | Outputs the exact same signal received at the IN port. The signal is passed through with no scaling, clipping, or processing of any kind. |

## Parameters

None.

## Details

### Signal Flow

The GADebug module has the simplest possible signal processing path:

1. **Input Read**: Each sample, the module reads the voltage present at the IN port (port ID 0) using the standard `getInput()` helper, which returns channel 0 of the signal. If nothing is connected, this returns 0.0.

2. **Display Capture**: The raw input voltage is stored in the `displayValue` field on the DSP side. This value is then synchronized back to the UI module via the `syncFromDSP` path, where it is stored as `debugValue` for rendering on the module's visual panel. The display shows the actual voltage (in the +/-5V range for audio-rate signals or whatever voltage is present), with no scaling or normalization applied.

3. **Pass-Through Output**: The input voltage is written directly to the OUT port (port ID 1) with no transformation. The output is bit-identical to the input.

### UI/DSP Synchronization

Unlike most modules in the GrooveboxAdvanced system, GADebug has a reverse synchronization direction. Most modules sync parameters from the UI to the DSP (`syncToDSP`), but GADebug syncs a value from the DSP back to the UI (`syncFromDSP`). The `syncToDSP` method is intentionally empty because GADebug has no user-adjustable parameters. The `syncFromDSP` method copies the `displayValue` from the DSP module to the `debugValue` field on the UI module, where the panel renderer can display it.

### Stateless Design

The `displayValue` is transient and is not serialized. When the module is saved and reloaded, `debugValue` starts at 0.0 and is immediately updated from the live signal on the next audio cycle. The `reset()` method zeros out `displayValue`.

### Visual Sizing

The GADebug module is wider than standard modules (2x width) to accommodate the value display on its panel.

## Tips

- Insert GADebug between any two modules to inspect the voltage at that point in the signal chain. Because it is a transparent pass-through, it can be left in place without affecting the audio or CV signal.
- Use GADebug to verify that an envelope or LFO is producing the expected voltage range. Patch the modulation source into GADebug's IN, then patch GADebug's OUT to the intended destination.
- When troubleshooting silent patches, place GADebug at successive points along the signal path to identify where the signal drops to zero or diverges from the expected value.
- Use GADebug on the output of a sequencer or quantizer module to confirm the exact voltage being generated at each step, which is helpful when tuning pitch sequences.
- Multiple GADebug modules can be used simultaneously at different points in a patch to compare signal values across the chain.
