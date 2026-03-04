# GAEffectsOut

The final destination module for the Effects global patch. GAEffectsOut collects processed audio from the effects chain and delivers it back to the GrooveboxAdvanced voice system, which then routes it to the hardware output jacks. It is the counterpart to GAEffectsIn: where GAEffectsIn injects the mixed step audio into the effects patch, GAEffectsOut captures the result after any effects processing modules in between.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN1 | Audio | Processed audio channel 1. Receives audio at the internal +/-5V scale and normalizes it to +/-1 for the voice output stage. |
| 1 | IN2 | Audio | Processed audio channel 2. Same behavior as IN1. |
| 2 | IN3 | Audio | Processed audio channel 3. Same behavior as IN1. |
| 3 | IN4 | Audio | Processed audio channel 4. Same behavior as IN1. |
| 4 | IN5 | Audio | Processed audio channel 5. Same behavior as IN1. |
| 5 | IN6 | Audio | Processed audio channel 6. Same behavior as IN1. |
| 6 | IN7 | Audio | Processed audio channel 7. Same behavior as IN1. |
| 7 | IN8 | Audio | Processed audio channel 8. Same behavior as IN1. |

## Outputs

None.

## Parameters

None.

## Details

### Signal Flow

GAEffectsOut sits at the end of the Effects global patch signal chain. The full signal path is:

1. **Step patches produce audio**: Each active step's patch generates audio through its own modules (carriers, modulators, filters, etc.), summed through GAOutput modules into 8 channels.
2. **Mixed audio enters the Effects patch**: The GrooveboxAdvanced main processor normalizes the mixed step audio from +/-5V to +/-1 and feeds it into GAEffectsIn modules, which scale it back to +/-5V for internal patch processing.
3. **Effects processing**: Between GAEffectsIn and GAEffectsOut, the user can place any combination of effects modules (delay, comb filter, filters, etc.) to process the audio.
4. **GAEffectsOut collects the result**: Each input port reads its signal at the internal +/-5V scale and divides by 5.0 to normalize back to +/-1.
5. **Voice output stage**: The GAProcessor's `getOutputs()` method reads the `processedAudio` array from each GAEffectsOut module, scales it back to +/-5V, and sums it into the final output bus. The result then passes through a `tanh` soft limiter before reaching the hardware output jacks.

### DSP Processing

The `process()` method is minimal:

```
processedAudio[i] = getInput(EFFECTSOUT_INx) / 5.0f
```

Each of the 8 input ports is read and divided by 5.0 to convert from the internal +/-5V audio scale to a normalized +/-1 range. This normalized audio is stored in the `processedAudio` array, which the voice processor reads after the effects patch has been fully processed.

### Output Summation

If multiple GAEffectsOut modules exist in the same Effects patch, their `processedAudio` arrays are summed together by the GAProcessor. This allows splitting the effects chain into parallel paths that recombine at the output.

### Relationship to GAOutput

GAEffectsOut serves a similar role to GAOutput but specifically for the Effects global patch. GAOutput is used in step patches (library patches) to route audio out of individual steps. GAEffectsOut is used in the Effects global patch to route audio out of the effects chain. Both contribute to the final output bus via the GAProcessor's `getOutputs()` method.

### No Parameters or Outputs

GAEffectsOut has no user-configurable parameters and no output ports. It is strictly an audio sink -- the terminal node of the Effects patch. The `syncToDSP()` method is a no-op since there is nothing to synchronize.

## Tips

- Place a single GAEffectsOut at the end of your effects chain and cable the processed audio from your last effects module into its IN ports. At minimum, connect IN1 and IN2 for stereo output.
- For parallel effects processing, use multiple signal paths from GAEffectsIn to a single GAEffectsOut. For example, split the dry signal and a delayed signal into separate processing chains, then connect both to the same GAEffectsOut inputs via a mixer.
- If you need parallel effects that should be summed independently, you can use multiple GAEffectsOut modules. Their outputs are summed automatically by the voice processor.
- The channels map directly to GrooveboxAdvanced's 8 hardware output jacks. IN1 corresponds to OUTPUT 1, IN2 to OUTPUT 2, and so on. Use this to maintain multi-channel routing through the effects stage.
- If the Effects global patch is empty (no modules at all), the effects stage is bypassed entirely and the mixed step audio goes directly to the hardware outputs. You only need GAEffectsIn and GAEffectsOut when you want to process the mixed audio before it reaches the outputs.
- Keep in mind that the final output passes through a tanh soft limiter after the effects stage. If your effects chain adds significant gain, the limiter will provide smooth saturation rather than harsh clipping.
