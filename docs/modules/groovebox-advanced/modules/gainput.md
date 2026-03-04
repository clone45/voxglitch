# GAInput

A bridge module that brings external signals from the VCV Rack host into the GrooveboxAdvanced internal patch environment. It maps the six CV input jacks on the main GrooveboxAdvanced panel to six output ports inside a patch, enabling external CV, audio, gates, and polyphonic signals to interact with the internal patch modules.

## Inputs

None. The GAInput module has no internal input ports. Instead, it receives its signals directly from the six CV input jacks (CV 1 through CV 6) on the main GrooveboxAdvanced host module panel. These are injected by the DSP engine before each processing cycle.

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | 1 | Control (CV) | Outputs the signal from host CV Input 1. Polyphonic, carrying up to 16 channels. The internal normalized value is scaled by 5.0, restoring the original VCV Rack voltage range. |
| 1 | 2 | Control (CV) | Outputs the signal from host CV Input 2. Polyphonic, carrying up to 16 channels. |
| 2 | 3 | Control (CV) | Outputs the signal from host CV Input 3. Polyphonic, carrying up to 16 channels. |
| 3 | 4 | Control (CV) | Outputs the signal from host CV Input 4. Polyphonic, carrying up to 16 channels. |
| 4 | 5 | Control (CV) | Outputs the signal from host CV Input 5. Polyphonic, carrying up to 16 channels. |
| 5 | 6 | Control (CV) | Outputs the signal from host CV Input 6. Polyphonic, carrying up to 16 channels. |

## Parameters

None. The GAInput module has no user-configurable parameters. It acts as a pure pass-through from host to patch.

## Details

### Signal Flow

The GAInput module forms the first stage of external signal ingestion for each patch. The complete signal path is:

1. **Host Input Reading**: The main GrooveboxAdvanced module reads polyphonic voltages from its six CV input jacks (CV 1 through CV 6). Each jack supports up to 16 polyphonic channels.

2. **Normalization**: The host divides each channel's voltage by 5.0, converting from VCV Rack's standard +/-5V bipolar range to an internal normalized +/-1.0 range. This normalization is performed in GrooveboxAdvanced.hpp before the values are passed into the patch engine.

3. **Distribution**: The GAProcessor finds all GAInput DSP modules in the patch and copies the six normalized PolySignal values into each module's `externalInputs[]` array. If a patch contains multiple GAInput modules, they all receive the same input data.

4. **Output Scaling**: During `process()`, the GAInputDSP module iterates over all six output ports. For each port, it copies the channel count and per-channel values from the corresponding `externalInputs` entry, multiplying each channel value by 5.0 to restore the original VCV Rack voltage scale. This means a +5V signal patched into the host jack will appear as +5V at the GAInput output port inside the patch.

### Polyphonic Behavior

All six output ports are poly-capable. The channel count on each output matches the channel count of the corresponding host CV input jack. If a mono cable is patched into host CV Input 1, output port 1 will carry 1 channel. If a 4-channel polyphonic cable is patched in, the output will carry 4 channels. When a host input jack has no cable connected, the output defaults to a single channel at 0V.

### Stateless Design

The module has no internal state beyond the `externalInputs[]` array, which is overwritten every sample by the processor. The `reset()` method clears all external input signals to zero. There are no parameters to sync between the UI and DSP layers -- `syncToDSP` and `syncFromDSP` are both no-ops because the input data is injected directly by GAProcessor rather than coming from UI controls.

## Tips

- Use GAInput to bring external LFOs, envelopes, sequencers, or any other CV source from VCV Rack into your GrooveboxAdvanced patches. This lets you modulate internal parameters like filter cutoff, oscillator pitch, or mixer levels from outside the groovebox.
- Patch a polyphonic V/Oct signal into one of the host CV inputs and connect the corresponding GAInput output to a VCO or Voice module's pitch input to play the internal synthesizer polyphonically from an external keyboard or sequencer.
- Use one input for a gate/trigger signal and another for pitch CV to drive internal envelope and oscillator modules, effectively turning a GrooveboxAdvanced patch into a playable voice controlled from the VCV Rack patch environment.
- Since every GAInput module in a patch receives the same six signals, you can place multiple GAInput modules at different locations in your patch canvas for convenient wiring without signal loss or duplication concerns.
- Pair with a GAAtten or GAScale module after the GAInput output to adjust the incoming signal's range to match what the destination module expects.
- The six inputs are general-purpose. You can mix uses freely: for example, use inputs 1-2 for pitch and gate, input 3 for an external clock, and inputs 4-6 for modulation CV sources.
