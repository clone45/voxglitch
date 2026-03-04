# GAVCA

A voltage-controlled amplifier (VCA) that scales an input signal's amplitude based on a combination of a base level knob and an optional CV modulation input. It follows the standard VCA paradigm: when no CV is connected, the LEVEL knob acts as a static gain control; when CV is connected, the CV signal multiplies with the base level to produce dynamic amplitude modulation.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | The primary signal input whose amplitude will be controlled. Accepts any signal type (audio, CV, gates, etc.). |
| 1 | CV | Control (CV) | Unipolar CV input (0-10V) that modulates the amplitude. The CV voltage is normalized from 0-10V to 0-1 and then multiplied with the base LEVEL value. Negative CV values are clamped to zero, so the CV cannot invert the signal. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | The gain-controlled output signal: `IN * effectiveGain`. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| LEVEL | Knob | 0.0 - 1.0 | 1.0 | Base gain level. At 1.0 the input passes through at unity gain (when no CV is connected) or at the full CV-determined amplitude. At 0.0 the output is silent regardless of CV. Intermediate values scale proportionally. |

## Details

### Signal Flow

The processing is stateless and computed each sample:

1. **Input Acquisition**: The audio input signal and CV signal are read from their respective ports.

2. **Gain Calculation (CV Connected)**: When a cable is connected to the CV input, the effective gain is computed as the product of the LEVEL knob and the normalized CV value. The CV voltage is first clamped so that negative values become zero, then divided by 10.0 to map the standard VCV Rack 0-10V unipolar range to a 0-1 gain multiplier. The formula is: `effectiveGain = level * max(0, cv) / 10.0`.

3. **Gain Calculation (CV Not Connected)**: When no cable is connected to the CV input, the LEVEL knob value is used directly as the gain: `effectiveGain = level`. This makes the module behave as a simple static volume control.

4. **Output**: The input signal is multiplied by the effective gain and sent to the output: `output = input * effectiveGain`.

### CV Modulation Behavior

The CV input uses VCV Rack's standard 0-10V unipolar convention. Key characteristics:

- At LEVEL 1.0 with a 10V CV signal, the output equals the input (unity gain).
- At LEVEL 1.0 with a 5V CV signal, the output is half the input amplitude.
- At LEVEL 0.5 with a 10V CV signal, the output is half the input amplitude.
- Negative CV voltages are clamped to zero, so the VCA cannot invert the signal or produce negative gain. This ensures clean amplitude control without unexpected phase flips.
- The LEVEL knob effectively sets the ceiling for the CV-controlled gain. A LEVEL of 0.5 means the maximum possible gain is 0.5, achieved only when CV reaches 10V.

## Tips

- Patch an envelope generator's output into the CV input to shape note dynamics. Set LEVEL to 1.0 so the envelope has full control over the amplitude contour.
- Use an LFO patched into the CV input to create a tremolo effect. Ensure the LFO output is unipolar (0-10V); a bipolar LFO will have its negative half clamped to zero, producing a choppy half-wave tremolo rather than a smooth one.
- Use the LEVEL knob as a simple volume control when no CV is connected. This is handy for balancing levels between different signal paths in a patch.
- Chain a GAVCA after a mixer to apply a master volume envelope to the combined signal before routing to the output module.
- For velocity-sensitive patches, use the CV input to receive a velocity CV signal so that louder notes produce higher amplitude and quieter notes are attenuated.
- Combine with a GAAtten module feeding the CV input to rescale or offset a modulation source before it reaches the VCA, giving finer control over the modulation depth.
