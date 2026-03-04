# GARingMod

A ring modulator that multiplies two audio signals together, producing sum and difference frequencies. Ring modulation creates metallic, bell-like, and inharmonic timbres by generating new spectral content based on the frequency relationship between the two input signals.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | A | Audio | First audio input signal. This signal is multiplied with the signal at input B to produce the ring modulated output. |
| 1 | B | Audio | Second audio input signal. This signal is multiplied with the signal at input A. The two inputs are symmetric: swapping A and B produces the same result. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | Mono audio output. The product of the A and B input signals, soft-clamped to the -10V to +10V range. |

## Parameters

None. GARingMod has no user-configurable parameters. The module performs a direct multiplication of its two inputs with no additional controls.

## Details

### Signal Flow

1. **Input**: Two audio signals are read from the A and B input ports.

2. **Multiplication**: The two signals are multiplied together sample-by-sample (`output = A * B`). This is the core ring modulation operation. When both inputs are zero, the output is zero. When only one input is connected and the other receives no signal (defaulting to 0.0), the output will be silent.

3. **Clamping**: The product is soft-clamped to the -10.0 to +10.0 range using `rack::math::clamp`. This prevents extreme output values that could occur when both inputs carry high-amplitude signals (for example, two 10V signals multiplied together would produce 100V without clamping).

4. **Output**: The clamped result is written to the OUT port.

### Spectral Behavior

Ring modulation produces sum and difference frequencies from the input signals. When input A is a sine wave at frequency `f1` and input B is a sine wave at frequency `f2`, the output contains two new frequencies: `f1 + f2` (sum) and `|f1 - f2|` (difference). The original frequencies `f1` and `f2` are not present in the output.

- **Harmonic relationships**: When the two input frequencies are harmonically related (for example, octaves or fifths), the sum and difference frequencies also fall on harmonic intervals, producing musical tones.
- **Inharmonic relationships**: When the two input frequencies are not harmonically related, the sum and difference frequencies form inharmonic partials, creating metallic, bell-like, or clangorous timbres characteristic of ring modulation.
- **Complex waveforms**: When the inputs contain multiple harmonics (as with sawtooth, square, or other complex waveforms), every harmonic of one signal is multiplied with every harmonic of the other, producing a dense spectrum of sum and difference tones.

### Amplitude Modulation Connection

Ring modulation is closely related to amplitude modulation (AM). The difference is that AM includes the original carrier signal in the output, while ring modulation does not. To approximate AM synthesis, mix the ring-modulated output with one of the original input signals using a GAMixer.

### No State

GARingMod is a purely combinational module with no internal state. It produces output based solely on its current input values with no memory of previous samples. There is no reset behavior.

## Tips

- Patch two oscillators at different frequencies into the A and B inputs to create classic ring modulation timbres. Slight detuning between the oscillators produces slowly shifting spectral beating effects.
- Use a low-frequency oscillator (GALFO) as one input and an audio-rate oscillator as the other to create tremolo-like amplitude modulation effects. The depth of the tremolo is determined by the LFO's amplitude.
- For bell-like or metallic percussion, patch two sine wave oscillators with an inharmonic frequency ratio (such as 1:1.4 or 1:2.76) and shape the result with a GAEnvelope through a GAVCA.
- To add movement to a static timbre, modulate the frequency of one oscillator input with an LFO or envelope while keeping the other fixed. The shifting sum and difference frequencies create evolving spectral animation.
- Chain GARingMod with a GAFilter to tame the often harsh and unpredictable harmonics. A low-pass filter can smooth out the higher sum frequencies while preserving the character of the difference tones.
- Use GARingMod as a voltage-controlled amplifier in a pinch: if one input carries audio and the other carries a unipolar control signal (such as an envelope output), the audio is amplitude-scaled by the control signal. This works because multiplication by a value between 0 and 1 attenuates the signal proportionally.
- Because both inputs must be non-zero for any output, GARingMod naturally gates audio. If either input drops to zero, the output is immediately silent with no tail or release.
