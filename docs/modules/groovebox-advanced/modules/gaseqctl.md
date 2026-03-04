# GASeqCtl

A sequencer control module that receives an external step index via CV and overrides the GrooveboxAdvanced host sequencer's step position. It lives in the Sequencer Control global patch and allows external modules or internal patch logic to directly dictate which sequencer step is active, bypassing the host's built-in clock-driven step advancement. The input is polyphonic: each polyphonic channel independently controls the step position for the corresponding patch in the patch library.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | STEP | Control (CV) | Polyphonic step index input. Each channel's voltage is clamped to the range 0.0 to 63.0 (corresponding to sequencer steps 0 through 63). When the input is monophonic (1 channel), channel 0's value is broadcast to all 16 patch slots. When polyphonic, each channel independently sets the step position for the corresponding patch index. |

## Outputs

None.

## Parameters

None.

## Details

### Signal Flow

The GASeqCtl module operates as a bridge between CV signals in the Sequencer Control global patch and the host GrooveboxAdvanced sequencer engine. The signal path is:

1. **CV Input Reception**: The STEP input port receives a polyphonic CV signal. Each of the up to 16 channels represents a step index for one patch slot in the patch library.

2. **Value Clamping**: In the `process()` method, the module iterates over all 16 possible patch channels. For each channel, it reads the corresponding polyphonic input value using `PolySignal::get(c)` and clamps it to the range `[0.0, PATCH_SEQ_STEPS - 1]` (i.e., 0.0 to 63.0). The clamped values are stored in the `stepValues[]` array and the polyphonic channel count is recorded.

3. **Mono Broadcasting**: The `PolySignal::get(c)` method handles mono-to-poly broadcasting automatically. When the input has only 1 channel, requesting any channel index returns channel 0's value. This means a single mono CV controls all 16 patch slots uniformly.

4. **Host Consumption**: After the Sequencer Control patch processes, the `GAProcessor::getSeqCtlOutputs()` method reads the first GASeqCtlDSP module it finds and copies the `stepValues[]` array and `channelCount` into a `SeqCtlOutputs` struct. The host's `process()` method then uses these values to override the sequencer step for each voice/patch, replacing the internal clock-driven step position.

### Step Override Behavior

When a GASeqCtl module exists in the Sequencer Control patch, the host activates "SeqCtl step mode." In this mode:

- **Legacy/Mono mode**: Channel 0's step value is used as the single `seqCtlStep` for global sequencer position.
- **Per-patch mode**: When processing individual patches in the patch library, each patch index `i` reads `seqCtlOutputs.steps[i]` to determine its own independent step position. The value is cast to an integer and clamped to `[0, PATCH_SEQ_STEPS - 1]`.

This means with a polyphonic input, different patches can play different steps simultaneously, enabling polyrhythmic or independently sequenced behavior across the patch library slots.

### Stateless Design

The module has no configurable parameters and no state that persists between samples beyond the current `stepValues[]` array, which is overwritten every sample. The `syncToDSP` method is a no-op because the module is input-only with no UI parameters to sync. The `syncFromDSP` method is also a no-op because no DSP state needs to propagate back to the UI layer. The `reset()` method zeros all step values and resets the channel count to 1.

## Tips

- Place this module in the Sequencer Control global patch (not in regular step patches). It only functions when the host's Sequencer Control processing path discovers it.
- To create a simple external step sequencer, patch a stepped CV signal (0-63 range) from an external VCV Rack module through the host's inputs into this module's STEP port. Each voltage level selects a different sequencer step.
- Use a polyphonic signal source to give each patch in the patch library its own independent step position. For example, use multiple sequencers or a polyphonic sequencer module to drive different patches through different step patterns simultaneously.
- Combine with GAClock, GACounter, or GASequencer modules inside the Sequencer Control patch to build complex step-selection logic entirely within GrooveboxAdvanced, without needing external modules.
- Since the step value is a continuous float that gets cast to an integer by the host, fractional values are truncated. A value of 3.7 selects step 3. Use integer-valued CV sources for predictable step selection.
- When the STEP input is left unconnected, the step values default to 0.0 for all channels, which locks the sequencer on step 0. Always provide a signal to the STEP input when using this module.
- For mono operation where all patches should follow the same step, simply connect a single monophonic cable. The mono value automatically broadcasts to all 16 patch slots.
