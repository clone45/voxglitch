# GAMixer

A 3-input audio mixer that combines up to three signals with individual level controls and a master output level. It serves as a basic summing utility for combining oscillators, effects returns, or any other audio or CV signals within a patch.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN 1 | Audio | First signal input. Scaled by the IN 1 level knob before summing. |
| 1 | IN 2 | Audio | Second signal input. Scaled by the IN 2 level knob before summing. |
| 2 | IN 3 | Audio | Third signal input. Scaled by the IN 3 level knob before summing. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | OUT | Audio | The mixed output signal: the sum of all three level-scaled inputs, multiplied by the master level. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| IN 1 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 1. At 1.0 the signal passes at unity; at 0.0 the input is fully muted. |
| IN 2 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 2. At 1.0 the signal passes at unity; at 0.0 the input is fully muted. |
| IN 3 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 3. At 1.0 the signal passes at unity; at 0.0 the input is fully muted. |
| MSTR | Knob | 0.0 - 1.0 | 1.0 | Master output level applied after the three inputs are summed. At 1.0 the summed signal passes at full level; at 0.0 the output is silent. |

## Details

### Signal Flow

The GAMixer is a stateless summing module. Each sample is processed independently with no internal memory, filtering, or feedback:

1. **Input Scaling**: Each of the three input signals is divided by 5.0 to convert from VCV Rack's standard +/-5V range to a normalized +/-1.0 internal range, then multiplied by the corresponding per-channel level knob (0.0 to 1.0).

2. **Summing**: The three scaled signals are added together. Because all three channels default to unity and operate on normalized values, the sum can reach up to +/-3.0 in the internal domain when all three inputs carry full-scale signals.

3. **Master Level**: The summed signal is multiplied by the master level knob (MSTR), providing a final gain control over the combined output.

4. **Output Scaling**: The result is multiplied by 5.0 to convert back to VCV Rack's +/-5V audio standard before being written to the OUT port.

### Gain Structure

With all knobs at their defaults (1.0), the mixer is a simple unity-gain sum: three +/-5V inputs produce a +/-15V output at maximum. This means the mixer can exceed VCV Rack's nominal +/-5V range when multiple hot signals are summed. Use the per-channel levels or master level to tame the output if clipping or distortion is a concern downstream.

The per-channel knobs are unipolar (0.0 to 1.0), so they can only attenuate -- they cannot boost or invert a signal. For signal inversion or bipolar scaling, use a GAAtten module before the mixer input.

## Tips

- Use the GAMixer to combine multiple oscillators (e.g., a Carrier and a VCO) into a single audio stream before routing to a Filter or Output module.
- Pull the per-channel levels below 1.0 to create a rough mix balance between voices or layers without needing separate VCA modules.
- Use the MSTR knob as a quick fade-out or overall volume trim for a submix without touching individual channel levels.
- For more than three inputs, chain two GAMixer modules: feed the output of the first into one input of the second, leaving two additional inputs free. Alternatively, use a GAMixer4, GAMixer6, or GAMixer8 module for larger submixes.
- Because the mixer is stateless and introduces no latency or coloration, it works equally well for mixing CV signals (e.g., combining multiple LFO outputs into a single complex modulation source) as it does for audio.
- When summing signals causes the output to exceed +/-5V, use the MSTR knob to scale the combined signal back into a safe range, or follow the mixer with a GAAtten or GAAmp module for more precise level management.
