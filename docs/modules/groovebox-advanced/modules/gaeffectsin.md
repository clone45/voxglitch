# GAEffectsIn

An audio source module that provides the entry point for audio into the global Effects patch. It receives the mixed audio output from all active step patches (up to 8 channels) and makes it available as output ports within the Effects patch for further processing, routing, or mixing before final output.

## Inputs

None.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | OUT1 | Audio | Mixed audio channel 1 from all active step patches, scaled to +/-5V. |
| 1 | OUT2 | Audio | Mixed audio channel 2 from all active step patches, scaled to +/-5V. |
| 2 | OUT3 | Audio | Mixed audio channel 3 from all active step patches, scaled to +/-5V. |
| 3 | OUT4 | Audio | Mixed audio channel 4 from all active step patches, scaled to +/-5V. |
| 4 | OUT5 | Audio | Mixed audio channel 5 from all active step patches, scaled to +/-5V. |
| 5 | OUT6 | Audio | Mixed audio channel 6 from all active step patches, scaled to +/-5V. |
| 6 | OUT7 | Audio | Mixed audio channel 7 from all active step patches, scaled to +/-5V. |
| 7 | OUT8 | Audio | Mixed audio channel 8 from all active step patches, scaled to +/-5V. |

## Parameters

None.

## Details

### Signal Flow

1. **Source**: The GrooveboxAdvanced main process loop sums the audio outputs from all active step patches (or patch library slots) into an 8-channel mix. Each step patch can produce up to 8 channels of audio via its own GAOutput module.

2. **Normalization**: Before being passed to the GAEffectsIn module, the mixed audio (which is at +/-5V scale from the step patches) is normalized to a +/-1 range by dividing by 5.0.

3. **Injection**: The normalized audio is written into the GAEffectsInDSP module's internal `mixedAudio[8]` array by the voice processor before the Effects patch is processed.

4. **Scaling**: During `process()`, each channel is scaled back up to the +/-5V internal patch signal range by multiplying by 5.0. This restores the audio to the standard signal level used by all other modules within the patch.

5. **Output**: The scaled audio is set on the 8 output ports, making it available for connection to other modules within the Effects patch via internal cables.

### Role in the Effects Patch

The GAEffectsIn module is exclusively used within the global Effects patch. It acts as the bridge between the per-step sound generation pipeline and the global post-processing stage. The typical signal chain is:

- Step patches generate audio via their own modules and route it to GAOutput modules.
- All step outputs are summed together.
- The summed audio enters the Effects patch through GAEffectsIn.
- Within the Effects patch, the audio can be processed by any modules (filters, delays, distortion, mixing, etc.).
- The processed audio exits the Effects patch through a GAEffectsOut module.
- The final output is soft-limited via `tanh` and sent to the GrooveboxAdvanced hardware output ports.

If the Effects patch contains no modules, the mixed step audio bypasses this stage entirely and goes directly to the hardware outputs.

### Multi-Channel Architecture

The 8 output channels correspond to the 8 hardware output ports of the GrooveboxAdvanced module. Each step patch can route its audio to any combination of these 8 channels via its own GAOutput module. The GAEffectsIn module preserves this channel separation, allowing the Effects patch to process channels independently or mix them together as needed.

### No Parameters or Sync

The GAEffectsIn module has no user-adjustable parameters. Its `syncToDSP` and `syncFromDSP` methods are no-ops because the audio data is injected directly by the voice processor rather than being synced from UI state.

## Tips

- The GAEffectsIn module is the starting point for building a global effects chain. Connect its outputs to effect modules (such as GADelay, GAFilter, GADistortion, or GAComb) and then route the processed signal into a GAEffectsOut module to complete the chain.
- Use the 8 separate output channels to apply different effects to different groups of instruments. For example, route drums (channels 1-2) through a compressor while routing synth pads (channels 3-4) through a reverb or delay.
- For a simple pass-through configuration, connect each GAEffectsIn output directly to the corresponding GAEffectsOut input. This preserves the unprocessed step audio at the final outputs.
- To create a master effects bus, use a GAMixer to combine selected GAEffectsIn channels, process the mix through shared effects, and then route the result to the desired GAEffectsOut channels.
- Remember that audio arriving at this module is already the sum of all active step patches. If individual steps are playing simultaneously, their signals are mixed before reaching GAEffectsIn. Use per-step output channel routing to keep sounds separated if you need independent effects processing.
- If the Effects patch is left empty (no modules), the mixed step audio passes directly to the hardware outputs without any processing. Add a GAEffectsIn and GAEffectsOut pair as a minimum to enable the effects processing path.
