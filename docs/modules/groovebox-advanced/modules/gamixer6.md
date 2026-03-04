# MIXER 6

A 6-input audio mixer that sums up to six audio signals into a single output, with individual level controls per channel and a master output level.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| MIXER6_IN1 | IN 1 | Audio | First audio input signal |
| MIXER6_IN2 | IN 2 | Audio | Second audio input signal |
| MIXER6_IN3 | IN 3 | Audio | Third audio input signal |
| MIXER6_IN4 | IN 4 | Audio | Fourth audio input signal |
| MIXER6_IN5 | IN 5 | Audio | Fifth audio input signal |
| MIXER6_IN6 | IN 6 | Audio | Sixth audio input signal |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| MIXER6_OUT | OUT | Audio | Summed and level-scaled audio output |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| IN 1 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 1 |
| IN 2 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 2 |
| IN 3 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 3 |
| IN 4 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 4 |
| IN 5 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 5 |
| IN 6 | Knob | 0.0 - 1.0 | 1.0 | Level control for input 6 |
| MSTR | Knob | 0.0 - 1.0 | 1.0 | Master output level applied after summing all channels |

## Details

The GAMixer6 is a stateless summing mixer with six input channels. The signal flow works as follows:

1. Each input signal is received at VCV Rack standard levels (up to +/-5V) and immediately divided by 5.0 to normalize it into the +/-1V internal processing range.
2. Each normalized input is then multiplied by its corresponding per-channel level parameter (IN 1 through IN 6), which scales from 0.0 (silent) to 1.0 (full level).
3. All six scaled signals are summed together into a single value.
4. The summed signal is multiplied by the master level parameter (MSTR).
5. The result is scaled back up by 5.0 to return to VCV Rack standard +/-5V range before being sent to the output.

Because all six channel levels default to 1.0 and the master level defaults to 1.0, a fully connected mixer with all knobs at their defaults will produce an output that is the direct sum of all six inputs (at unity gain per channel). If all six inputs carry full-scale +/-5V signals simultaneously, the summed output can reach up to +/-30V before master scaling, so reducing channel levels or the master level is recommended when mixing many hot signals to avoid clipping downstream.

The mixer has no internal state, filters, or slew limiting. Parameters are synced from the UI to the DSP module each processing cycle via `syncToDSP`, which copies all seven level values (six channels plus master) to the DSP module.

## Tips

- Use the per-channel knobs to set a rough balance between instruments or voices, then adjust the MSTR knob to control the overall volume going into downstream modules such as effects or the output module.
- When mixing fewer than six sources, unused inputs contribute nothing to the output (they read 0V), so there is no need to turn down unused channel knobs.
- Chain two MIXER 6 modules together (output of one into an input of the other) to mix more than six signals.
- For mixing drums and melodic elements separately, use one MIXER 6 for drums and another for melodic voices, then combine them through a final mixer or directly into the output module. This gives you sub-group level control.
- Since the channel levels are unipolar (0.0 to 1.0), this mixer attenuates but does not invert signals. If you need signal inversion, place an Attenuverter (ATTEN) module before the mixer input.
