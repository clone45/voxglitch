# Impulse Module Architecture

## Introduction

The Impulse module is a polyphonic Karplus-Strong synthesizer that supports up to 16 voices. The architecture separates excitation generation from the string simulation itself. This separation made the code easier to maintain, especially when adding the bytebeat excitation sources.

## Voice System

Impulse uses an array of 16 independent `ImpulseVoice` objects, each containing its own delay line, filters, and state. While this uses more memory than a shared buffer approach, it makes polyphony straightforward—when a V/Oct input comes in with multiple channels, the module activates as many voices as needed and each processes independently.

Each voice tracks whether it's actively synthesizing through a `noteActive` boolean flag. On trigger, the voice fills its delay line with energy from the selected excitation source, which then circulates through the feedback loop. Voices don't communicate with each other, which simplifies the implementation.

## DSP Chain

Each voice runs through the same processing chain:

**DelayLine**: A circular buffer (max 8192 samples) whose length determines pitch.

**DampingFilter**: A one-pole lowpass filter that creates the high-frequency decay characteristic of plucked strings.

**DCBlocker**: A high-pass filter that removes DC offset buildup in the feedback loop, which is necessary for stability.

**CombDelay**: An optional larger delay buffer (max 262144 samples, 32x the main delay line size) with feedback control for harmonic resonances.

## Excitation Sources

The module provides three excitation modes: Noise, Triggered Bytebeat, and Continuous Bytebeat. To organize this, there's a base class `ExcitationSource` with a virtual `generate()` method, and two implementations: `NoiseExcitation` and `BytebeatExcitation`. The mode switching logic uses a simple integer flag that gets checked in several places. This could be refactored to a more polymorphic approach if additional excitation sources are added.

**NoiseExcitation**: Generates random values between -1.0 and 1.0. It's completely stateless.

**BytebeatExcitation**: Implements 41 algorithms based on bitwise and arithmetic operations on a time counter. It provides methods to select an equation, set three modulation parameters, and generate samples at a controllable rate. Rate scaling normalizes to a 44.1 kHz reference before applying a user multiplier. A time offset parameter allows seeking within the bytebeat sequence.

## Shared Resources

The module creates only one instance of each excitation source, shared across all 16 voices. This works because the excitation sources are stateless—they generate samples on demand without maintaining per-voice state. This saves memory and is efficient, but constrains the design: any excitation source that needs to remember state between calls would need that state moved into the voice itself.

## Polyphony and CV Architecture

Polyphony is dynamic, determined by the V/Oct input channel count. The output channel count automatically matches the input.

The CV architecture is split: most parameters (damping, decay, bleed, bytebeat controls) use CV channel 0 and apply the same modulation to all voices. Each voice gets its own pitch from its corresponding V/Oct channel and has its own trigger detector. This provides independent triggering per voice while keeping synthesis parameter control unified. The alternative—per-voice CV for every parameter—would require many more inputs and would be harder to patch.

## Process Loop

Each audio frame follows this flow:

1. Handle mode buttons and LED updates
2. Calculate shared parameters (base pitch, damping, decay, etc.) by combining knob values with CV channel 0, scaled by attenuverters
3. Loop through active voices:
   - Calculate per-voice pitch (base pitch + V/Oct channel)
   - Convert to frequency using `exp2_taylor5()` approximation
   - Check trigger input via Schmitt trigger
   - On trigger: call `voice.exciteString()` with selected excitation source
   - Every sample: call `voice.process()` to generate audio
4. If in continuous bytebeat mode, increment the global time counter

## Implementation Choices

**Pitch Conversion**: Uses `exp2_taylor5()` instead of standard library `exp()` for better performance with acceptable accuracy.

**Feedback Calculation**: An analytical formula `exp(-6.91 / (frequency * decayTime))` provides perceptually consistent decay times across the pitch range.

**SafeMath Utility**: Bytebeat equations use division and modulo operations extensively. A small utility checks for zero divisors and returns safe defaults to prevent crashes.

**Memory Allocation**: All delay buffers use fixed-size stack allocation. The main delay line caps at 8192 samples, the comb delay at 262144 samples. This imposes a hard limit on the lowest pitch but eliminates memory allocation failures or fragmentation in the audio thread.

## Continuous Injection

The module supports continuous injection of excitation energy (called "bleed" in the UI) rather than just the initial trigger-based excitation typical of Karplus-Strong synthesis. The injection can occur at a controllable rate.

An optional rate sync feature links the injection rate to the fundamental pitch. When enabled, bytebeat patterns lock to the harmonic series of the note.

## State Persistence

The module serializes these settings to JSON:
- Active source mode
- Rate sync enabled state
- Current continuous bytebeat sequence position
- Comb delay clear-on-trigger flag
- Selected max rate option

The continuous time counter is saved to preserve patch textures across reloads, though this means patches may sound slightly different depending on where in the bytebeat sequence they were saved.

## File Organization

```
src/Impulse/
├── Impulse.hpp                      # Main module, parameter management, process loop
├── ImpulseVoice.hpp                 # Per-voice processing
├── DelayLine.hpp                    # Circular buffer
├── DampingFilter.hpp                # One-pole lowpass
├── DCBlocker.hpp                    # High-pass filter
├── CombDelay.hpp                    # Comb filter
├── SafeMath.hpp                     # Safe div/mod utilities
├── ImpulseComponents.hpp            # UI components
├── ImpulseWidget.hpp                # UI layout
├── ImpulseEquationReadoutWidget.hpp # Equation display
└── excitation/
    ├── ExcitationSource.hpp         # Virtual base class
    ├── NoiseExcitation.hpp          # White noise implementation
    └── BytebeatExcitation.hpp       # 41 bytebeat algorithms
```

The DSP components are kept in separate headers to make them easier to test and reuse. The excitation sources live in their own subdirectory with a base interface and concrete implementations.

## Design Considerations

The architecture's strength is separation of concerns: voices don't know about each other, excitation sources don't know about DSP, and DSP components are focused on single responsibilities. This made adding polyphony and continuous injection relatively straightforward.

The main area that could be improved is the source mode switching logic, which currently uses an integer checked in multiple places. A polymorphic approach would be cleaner but more complex. The trade-off might be worth revisiting if more excitation sources are added.
