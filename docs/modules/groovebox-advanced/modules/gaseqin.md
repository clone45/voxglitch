# GASeqIn

A utility module that exposes the host GrooveboxAdvanced module's external clock and reset signals as patchable outputs inside the internal patch. It provides access to both sequencer timing signals in a single module, enabling internal patch logic to respond to the host's clock and reset inputs.

## Inputs

None.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | CLK | Trigger | Outputs the external clock voltage received from the host GrooveboxAdvanced module's clock input jack. The voltage is passed through without modification, mirroring the exact voltage present at the host's clock input each sample. |
| 1 | RST | Trigger | Outputs the external reset voltage received from the host GrooveboxAdvanced module's reset input jack. The voltage is passed through without modification, mirroring the exact voltage present at the host's reset input each sample. |

## Parameters

None.

## Details

### Signal Flow

The GASeqIn module acts as a bridge between the host GrooveboxAdvanced module's external clock and reset inputs and the internal patch environment. The signal path is:

1. **Host Clock and Reset Inputs**: The GrooveboxAdvanced host module receives clock and reset signals on its external input jacks from the VCV Rack patch. The clock voltage is read from `inputs[CLOCK_INPUT].getVoltage()` and the reset voltage from `inputs[RESET_INPUT].getVoltage()`.

2. **Distribution by GAProcessor**: The `GAProcessor` calls `setSequencerInputs(clock, reset)` which iterates over all GASeqIn DSP modules in the patch and writes the current clock voltage to each module's `externalClock` field and the current reset voltage to each module's `externalReset` field. This happens every sample before the patch graph is processed.

3. **Pass-Through Output**: The `process()` method performs two operations: it copies `externalClock` to the CLK output port and `externalReset` to the RST output port using `setOutput()`. No scaling, filtering, or trigger detection is applied. The raw voltages are forwarded as-is.

### Stateless Design

The module has no internal state. The `reset()` method is empty because there is nothing to initialize. The `syncToDSP` method is a no-op because the `externalClock` and `externalReset` values are set directly by the GAProcessor rather than synced from the UI module. There is no `syncFromDSP` path since no DSP state needs to propagate back to the UI. The module has no configurable parameters.

### Relationship to GAVoice

In the Sequencer Control global patch, `GAVoice::processSeqControl()` calls `processor.setSequencerInputs(externalClock, externalReset)` before processing the patch graph. For per-step voice patches, `GAVoice::process()` and `GAVoice::processContinuous()` also pass the external clock and reset signals through to the processor. This means every GASeqIn module in any patch receives the same clock and reset voltages each sample.

### Comparison to GAReset and GAClock

GASeqIn combines the roles of GAReset (which provides only the reset signal) and the external clock path used by GAClock (which internally detects clock edges for sync mode). Unlike GAClock, GASeqIn does not perform any trigger detection, BPM calculation, or clock division/multiplication. It outputs the raw voltages, leaving trigger detection and timing logic to whatever modules receive the signal. Unlike GAReset, GASeqIn also provides the clock signal alongside reset in a single module.

## Tips

- Use GASeqIn when you need access to both the host clock and reset signals inside a patch. It is more compact than placing separate GAClock and GAReset modules.
- Patch the CLK output into ClockDiv or ClockMult modules to derive divided or multiplied clock signals from the host's clock for polyrhythmic patterns.
- Patch the RST output into sequencer or pattern generator reset inputs to synchronize their playback position with external reset signals.
- GASeqIn is particularly useful in the Sequencer Control global patch where you need to build custom sequencer logic that responds to the host's clock and reset signals, then feeds results into a SeqCtl module.
- Since both outputs are raw pass-throughs of host voltages, they work with any downstream module that expects trigger or gate signals. The receiving module is responsible for performing its own edge detection.
- Combine GASeqIn with Counter, ClockDiv, and Compare modules to build complex step-sequencing logic that derives its timing entirely from the host clock.
