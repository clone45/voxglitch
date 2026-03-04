# GAOutput

The final audio output destination for a GrooveboxAdvanced patch. GAOutput accepts up to eight audio signals on numbered input ports and applies a master level control before passing the results to the host processor. It has no output ports of its own -- the processor reads its internal output values directly to produce the audio that leaves the GrooveboxAdvanced module. Every patch that produces audio needs at least one GAOutput module.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | 1 | Audio | Audio input for channel 1. The signal is multiplied by the LEVEL knob and then stored as the channel 1 output for the host processor to read. |
| 1 | 2 | Audio | Audio input for channel 2. Processed identically to channel 1. |
| 2 | 3 | Audio | Audio input for channel 3. Processed identically to channel 1. |
| 3 | 4 | Audio | Audio input for channel 4. Processed identically to channel 1. |
| 4 | 5 | Audio | Audio input for channel 5. Processed identically to channel 1. |
| 5 | 6 | Audio | Audio input for channel 6. Processed identically to channel 1. |
| 6 | 7 | Audio | Audio input for channel 7. Processed identically to channel 1. |
| 7 | 8 | Audio | Audio input for channel 8. Processed identically to channel 1. |

## Outputs

None. GAOutput is the terminal node in a patch's signal chain. The host processor reads its internal output values directly rather than through explicit output ports.

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| LEVEL | Knob | 0.0 - 1.0 | 1.0 | Master output level applied to all eight channels. At 0.0 all channels are silent; at 1.0 signals pass through at unity gain. This single control scales every input channel uniformly before the processor reads the final values. |

## Details

### Signal Flow

1. **Input Reading**: On each sample, the module reads all eight input ports. Unconnected ports return 0V.

2. **Level Scaling**: Each input value is multiplied by the LEVEL parameter. Because the knob range is 0.0 to 1.0, this control can only attenuate -- it cannot boost a signal beyond its original amplitude. The same LEVEL value is applied to all eight channels uniformly.

3. **NaN/Inf Sanitization**: After the multiplication, each output value is checked for NaN and infinity. Any invalid floating-point value is replaced with 0.0, preventing corrupted signals from propagating to the host processor and causing downstream artifacts.

4. **Output Storage**: The processed values are stored in the module's internal output bank. The host processor reads these stored values via the `getOutputs()` method on GAProcessor after all modules in the patch have been processed.

### How the Processor Reads Output

The GAProcessor maintains a typed cache of all GAOutputDSP instances in the patch. When computing the final audio output, it iterates over every GAOutput module and sums their stored values channel by channel. This means multiple GAOutput modules in a single patch will have their values added together, allowing modular routing strategies where different signal paths terminate at separate output modules.

If a patch contains no GAOutput modules (and no GAEffectsOut modules), the processor falls back to summing Carrier module outputs on channel 0 as a legacy compatibility behavior.

### Soft Clipping and Limiting (Currently Disabled)

The DSP class contains commented-out code for two stages of output protection:

- **Soft clipping**: A fast tanh approximation that would normalize the signal to a unit range, apply tanh saturation, and scale back to +/-5V. This would provide smooth, musically useful saturation rather than hard digital clipping.
- **Hard clamping**: A simple voltage clamp to the +/-5V VCV Rack audio standard.

Both are currently disabled for signal analysis purposes. As a result, the output values are passed through without any amplitude limiting, and summing multiple hot signals can produce voltages exceeding +/-5V.

### Stateless Operation

GAOutput performs no sample-to-sample state tracking. Each output sample is computed purely from the current input samples and the current LEVEL knob value. The `reset()` method is intentionally empty because there is no internal state to clear.

## Tips

- Place a GAOutput module at the end of every patch that should produce audio. Without one, the patch will either be silent or fall back to the legacy Carrier-summing behavior, which only outputs on channel 0.
- Use the eight numbered inputs to map different signal sources to different physical output channels on the GrooveboxAdvanced module. For example, connect a kick drum to input 1 and a snare to input 2 to get independent outputs that can be mixed and processed separately outside the groovebox.
- When building a stereo patch, use inputs 1 and 2 as left and right channels. Route the left mix to input 1 and the right mix to input 2, then use the corresponding outputs on the GrooveboxAdvanced host module.
- The LEVEL knob acts as a master fader for the entire patch. Reduce it to lower the overall volume without adjusting individual mixer or VCA levels upstream. This is useful for balancing the volume of one sequencer step against another.
- If a patch uses multiple GAOutput modules, their per-channel values are summed by the processor. This can be used intentionally -- for instance, one GAOutput handles dry signals while another handles effect returns -- but be aware that the summed amplitude can exceed +/-5V with no built-in limiting.
- Because output protection is currently disabled, loud patches may clip downstream modules or the audio output. Use a GAMixer or GAVCA upstream to manage levels, or insert a GAAmp module with saturation before the GAOutput to provide soft clipping within the patch.
- Unconnected inputs return 0V and contribute nothing to the output. There is no need to cable every input -- use only the channels you need.
