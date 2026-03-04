# GAReset

A utility module that passes through the host GrooveboxAdvanced module's external reset signal into the internal patch. It allows internal patch modules (such as clocks, sequencers, and pattern generators) to synchronize with the external reset input on the main VCV Rack module.

## Inputs

None.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | OUT | Trigger | Outputs the external reset voltage received from the host GrooveboxAdvanced module's reset input jack. The voltage is passed through without modification, so it mirrors the exact voltage present at the host's reset input each sample. |

## Parameters

None.

## Details

### Signal Flow

The GAReset module acts as a bridge between the host GrooveboxAdvanced module's external reset input and the internal patch environment. The signal path is:

1. **Host Reset Input**: The GrooveboxAdvanced host module receives a reset signal on its external reset input jack from the VCV Rack patch.

2. **Distribution by GAProcessor**: The `GAProcessor` iterates over all GAReset DSP modules in the patch and writes the current reset voltage directly to each module's `externalResetTrigger` field via `setExternalReset()`. This happens every sample before the patch graph is processed.

3. **Pass-Through Output**: The `process()` method performs a single operation: it copies `externalResetTrigger` directly to the OUT port using `setOutput(RESET_OUT, externalResetTrigger)`. No scaling, filtering, or trigger detection is applied. The raw voltage is forwarded as-is.

### Stateless Design

The module has no internal state. The `reset()` method is empty because there is nothing to initialize. The `syncToDSP` method is a no-op because the `externalResetTrigger` value is set directly by the GAProcessor rather than synced from the UI module. There is no `syncFromDSP` path since no DSP state needs to propagate back to the UI. The module has no configurable parameters.

### Relationship to GAVoice

Each voice in GrooveboxAdvanced calls `processor.setExternalReset(externalReset)` before processing its patch graph. This means every GAReset module in the patch receives the same reset voltage each sample, regardless of which step or voice is active.

## Tips

- Patch the OUT port into clock modules' or sequencers' reset inputs to synchronize their playback position with the external reset signal coming into the host GrooveboxAdvanced module.
- Use GAReset alongside the GAClock module to build internally clocked patches that still respond to external reset signals for synchronization with the rest of your VCV Rack patch.
- Since the output is a raw pass-through of the host reset voltage, it works with both trigger-style resets (short pulses) and gate-style resets (sustained high signals), depending on what the receiving module expects.
- Place a GAReset module in any patch that uses internal sequencing or pattern generation to ensure those modules can be reset to their starting position from outside the GrooveboxAdvanced environment.
- When building the Sequencer Control global patch, use GAReset to feed the reset signal into sequencer control logic so that the entire sequencer state can be reset externally.
