# Spores - Granular Synthesis Module User Manual

## Overview

Spores is an advanced granular synthesis module for VCV Rack that transforms audio input or loaded samples into clouds of sonic particles called "grains." These grains can be manipulated, scattered, and processed to create everything from subtle textures to complex, evolving soundscapes.

The module operates in two main modes:
- **Live Input Mode**: Processes incoming audio in real-time
- **Sample Mode**: Works with loaded audio samples

## Quick Start Guide

Welcome to Spores! Getting started with granular synthesis is easier than you might think. Here are two simple ways to begin exploring:

### Getting Started as an Effects Processor

1. **Connect your audio**: Patch an audio source into the LEFT and/or RIGHT audio inputs on Spores (mono or stereo)
2. **Connect the output**: Patch the LEFT and RIGHT outputs to your mixer or audio interface
3. **Play your audio source** and you should hear the granular effect! With default settings, you'll hear the original sound mixed with scattered "grains" of itself
4. **Experiment**: Try adjusting DENSITY (more/fewer grains), SIZE (longer/shorter grains), and MIX (more wet signal for stronger effect)

### Getting Started with Sample Playback

1. **Load a sample**: Right-click on Spores and select "Load Sample" to browse for a WAV file
2. **Enable sample mode**: In the same right-click menu, make sure "Sample Mode" is checked
3. **Connect the output**: Patch the LEFT and RIGHT outputs to your mixer
4. **Basic playback**:
   - PITCH controls playback speed (12 o'clock = normal speed)
   - MIX controls how much of the original sample vs. granulated version you hear
   - Start with MIX at 50% to hear both
5. **Add granular magic**: Increase DENSITY to scatter the sample into grains, adjust SIZE for different grain lengths
6. **Try the WARP control**: This creates interesting time-stretching effects that separate pitch from timing

### Next Steps

Once you're comfortable with the basics:
- Explore the different visualization modes (right-click menu)
- Try connecting CV sources to the CV input jacks and adjusting their small attenuverter knobs
- Experiment with the SPREAD and PATTERN controls for rhythmic effects
- Check out the different Grain Schedulers in the right-click menu for various timing patterns

Don't worry about understanding everything at once - granular synthesis is all about experimentation and happy accidents!

## Panel Layout

The Spores module features a clean, organized layout with the following sections:

### Top Row Controls
- **MIX**: Blend between dry (original) and wet (processed) signal
- **DENSITY**: Controls how frequently new grains are created
- **SIZE**: Sets the duration/length of individual grains
- **COUNT**: Determines the maximum number of simultaneous grains
- **PITCH**: Adjusts the playback rate/pitch of grains
- **RND** (Pitch Variation): Adds random pitch variation to grains

### Middle Row Controls
- **JITTER**: Controls timing variation in grain creation
- **WARP**: Time-stretching effect that warps playback position
- **STEREO**: Adjusts stereo width and positioning of grains
- **FEEDBACK**: Routes processed output back into the input
- **SPREAD**: Distributes grains across time using various patterns
- **PATTERN**: Selects the specific spread distribution algorithm

### Central Visualizer
The large central display shows real-time visualization of grain activity and buffer contents with multiple display modes.

### Input/Output Section
- **Audio Inputs**: Left and right audio inputs with gain control
- **Audio Outputs**: Main stereo outputs plus 8 polyphonic outputs for individual grains
- **Special Inputs**: EMIT and INDEX for external control

### Status Indicators
- **Audio Input LED**: Shows incoming audio level and activity
- **Max Grains LED**: Illuminates when the grain pool is full (maximum grain count reached)

## Parameters and Controls

### Core Granular Parameters

#### MIX (0-100%)
Controls the balance between the original dry signal and the granulated wet signal.
- **0%**: Pure dry signal (bypass)
- **50%**: Equal mix of dry and wet
- **100%**: Pure granulated signal

#### DENSITY (0-100%)
Determines how frequently new grains are created.
- **Low values**: Sparse, individual grains
- **High values**: Dense clouds of overlapping grains
- **CV Input**: Accepts 0-10V control voltage
- **Attenuverter**: Bipolar control for CV amount

#### SIZE (0-100%)
Sets the duration of individual grains.
- **Small values**: Short, percussive grains
- **Large values**: Long, sustained grains
- **Range**: 2 to 48,000 samples (approximately 0.04ms to 1 second at 48kHz)

#### COUNT (2-128)
Controls the maximum number of grains that can play simultaneously.
- **Lower values**: Cleaner, less dense textures
- **Higher values**: More complex, chaotic textures
- **Note**: Actual maximum can be limited in the context menu
- **Max Grains LED**: The LED next to this control illuminates when the grain pool is full

#### PITCH (0.01x to 2x)
Adjusts the playback rate of grains.
- **0.5x**: One octave down
- **1.0x**: Original pitch
- **2.0x**: One octave up
- **CV Input**: 1V/octave standard

#### RND (Pitch Variation) (0-30%)
Adds random pitch variation to grains for more organic textures.
- **0%**: All grains at the same pitch
- **Higher values**: Increasingly random pitch variations

### Advanced Controls

#### JITTER (Variable Range)
Controls timing variation in grain creation.
- **Low values**: Precise, rhythmic grain timing
- **High values**: Chaotic, irregular grain timing

#### WARP (0-100%)
Creates time-stretching effects by warping the playback position.
- **50%**: No warping (normal playback)
- **Lower values**: Slow down effect
- **Higher values**: Speed up effect

#### STEREO (0-100%)
Controls stereo positioning and width of grains.
- **0%**: Mono (center position)
- **Higher values**: Wider stereo spread

#### FEEDBACK (0-130%)
Routes the processed output back into the input buffer. **Note: Only active in Live Input Mode - has no effect in Sample Mode.**
- **0%**: No feedback
- **Higher values**: Increasingly self-generating textures
- **>100%**: Can create unstable, chaotic feedback
- **Function**: Acts like an old tape delay, blending incoming audio with existing buffer content instead of overwriting it

#### SPREAD (0-100%)
Distributes grains across time using various pattern algorithms.
- **0%**: No spread (all grains at same position)
- **Higher values**: Increasingly scattered grain positions

#### PATTERN (0-17)
Selects the specific algorithm used for grain distribution.
- **0**: Incrementing - Sequential position progression
- **1**: Animating Incrementing - Smooth sequential movement
- **2**: Animating Incrementing Reverse - Reverse sequential movement
- **3**: Decrementing - Reverse sequential progression
- **4**: Pendulum - Back and forth motion
- **5**: Inside Out - Center to edges expansion
- **6**: Outside In - Edges to center contraction
- **7**: Dual Stream - Two parallel grain streams
- **8**: Dual Location Toggle - Alternating between two positions
- **9**: Triple Stream - Three parallel grain streams
- **10**: Pulse Wave - Rhythmic burst patterns
- **11**: Exponential - Exponentially spaced grains
- **12**: Animating Exponential - Moving exponential spacing
- **13**: Cluster Burst - Grouped grain bursts
- **14**: Breathing Cluster - Expanding/contracting clusters
- **15**: Micro Swarm - Small, dense grain groups
- **16**: Animating Kaleidoscope - Complex rotating patterns
- **17**: Random - Completely random positioning
- **CV Input**: Accepts 0-10V control voltage
- **Attenuverter**: Bipolar control for CV amount

### Direction Controls

#### Grain Direction Button/LED
- **Function**: Toggles forward/reverse playback for individual grains
- **LED**: Illuminated when reverse playback is active
- **CV Input**: Trigger input for external control

#### Buffer Direction Button/LED
- **Function**: Toggles forward/reverse buffer fill direction
- **LED**: Illuminated when reverse buffer fill is active
- **CV Input**: Trigger input for external control

### Special Inputs

#### EMIT Input
Controls grain creation timing when connected.
- **Trigger Mode**: Creates a grain on each rising edge
- **Stream Mode**: Creates continuous grains while signal is high
- **Voltage Range**: 0-10V

#### INDEX Input
Controls buffer position for grain creation.
- **Continuous Mode**: Directly sets buffer position (0-10V maps to full buffer)
- **Edge-Triggered Mode**: Updates position only when input changes
- **Use**: Precise control over where grains are created

## Operating Modes

### Live Input Mode
- Connect audio to the LEFT and/or RIGHT inputs (supports both mono and stereo)
- Adjust INPUT GAIN as needed
- The module continuously fills a buffer with incoming audio
- Grains are created from this live buffer

### Sample Mode
- Load a sample via the right-click context menu
- The loaded sample replaces the live buffer
- Grains are created from the sample data
- Sample playback position is controlled by PITCH parameter

## Visualization Modes

The central display offers multiple visualization modes accessible via the context menu:

### Lines
Shows grain activity as vertical lines representing grain positions and intensities. Features a grain-responsive waveform that highlights areas where grains are active with fade effects.

### Rectangles
Displays grains as rectangular blocks with width based on grain duration and height indicating grain envelope values.

### Spores
Unique spore-like particles that represent individual grains with organic movement. Spores grow in size and float vertically as grains progress through their lifecycle.

### Radial
Circular/radial representation where grain particles spawn around the edges and animate inward toward the center, creating a converging effect.

### Splash Screen
Shows module information and startup graphics with animated "VOXGLITCH SPORES" text. Can be manually selected or appears automatically when no audio ports are connected.

### Cosmic Dust
Particle-like visualization resembling cosmic dust clouds. Grains appear as stars with varying brightness, size, and orbital motion patterns that create a space-like atmosphere.

### Retro Vector Field
A retro-style visualization featuring green outlined particles that move through 3D space with perspective projection, creating a classic vector graphics aesthetic.

### None
Disables visualization for minimal CPU usage.

## Grain Schedulers

Spores includes multiple grain scheduling algorithms that control the timing and rhythm of grain creation. These can be selected from the context menu under "Grain Scheduler":

### Default (Fixed Interval)
The standard scheduler that creates grains at regular intervals based on the DENSITY parameter. Provides consistent, predictable grain timing suitable for most applications.

### Burst Taper
Creates rhythmic bursts of increased grain density every 4 seconds. During burst periods (3 seconds duration), grain density gradually increases to 3x normal density using a smooth bell curve, then returns to normal. Creates dramatic textural variations and rhythmic emphasis.

### Probabilistic
Adds organic variation to grain timing using probability:
- **70% chance**: Create grain at normal timing
- **20% chance**: Skip grain (creates gaps)
- **10% chance**: Create normal grain plus an extra grain nearby
Results in natural, breathing textures with subtle irregularities.

### Polyrhythmic
Creates complex rhythmic patterns using two overlapping grain streams:
- **Stream A**: Fires every 3 intervals
- **Stream B**: Fires every 5 intervals (slightly sparser)
The pattern repeats every 15 intervals, creating intricate polyrhythmic interactions suitable for rhythmic and percussive textures.

### Bouncing Ball
Simulates a bouncing ball settling pattern with accelerating grain creation:
- Starts with slow, widely-spaced grains
- Accelerates through a 15-step cycle to rapid-fire grains
- Resets and begins the cycle again
Creates dramatic build-ups and releases reminiscent of bouncing objects coming to rest.

### Bouncing Pendulum
Creates oscillating grain density patterns like a swinging pendulum:
- Gradually accelerates from slow to fast timing
- Reaches peak density at the center
- Decelerates back to slow timing
- Reverses direction and repeats
Produces breathing, wave-like rhythms with continuous oscillation between sparse and dense textures.

### Tides
Provides subtle, gentle modulation of grain density using a sine wave:
- **2-second cycle**: Complete oscillation period
- **±20% variation**: Modulates grain spacing around base density
- Creates gentle breathing/tidal effects without being obvious
Ideal for adding organic life to sustained textures without dramatic changes.

## Context Menu Options

Right-click the module to access advanced settings:

### Grain Settings

#### Grain Management
- **Grain Replacement Mode**: Choose how new grains behave when pool is full
  - Replace oldest grains when pool is full
  - Never replace active grains
- **Max Grain Pool Size**: Set maximum grain count (16, 32, 48, 64, 96, 128 grains)
- **Amplitude Compensation**: Dynamic range adjustment (Off/Low/Medium/High)

#### Grain Scheduler
- **Default (Fixed Interval)**: Standard regular timing
- **Burst Taper**: Rhythmic bursts with increased density
- **Probabilistic**: Organic variation with random timing
- **Polyrhythmic**: Complex overlapping rhythm patterns
- **Bouncing Ball**: Accelerating grain creation patterns
- **Bouncing Pendulum**: Oscillating density like a pendulum
- **Tides**: Gentle sine wave density modulation

#### Pitch Variation Mode
- **Microtonal (Subtle Variations)**: Small pitch deviations
- **Small Continuous (Medium Variations)**: Medium-range pitch changes
- **Descending**: Downward pitch progressions
- **Descending Wide**: Wide downward pitch ranges
- **Interval Jumps (Octave +/-)**: Octave-based pitch jumps
- **Interval Jumps (Octaves Down)**: Downward octave jumps
- **Interval Jumps (Fifths)**: Perfect fifth intervals

#### Grain Envelope Type
- **Triangle (Default)**: Linear attack and decay
- **Gaussian (Bell Curve)**: Smooth bell-shaped envelope
- **Rectangle (With Anti-click)**: Square envelope with click prevention
- **Sine**: Sine wave-shaped envelope
- **Hann Window**: Cosine-squared window function
- **Exponential**: Exponential attack/decay curves
- **Oscillating**: Envelope with internal oscillations
- **Oscillating (fast)**: Rapid oscillating envelope

#### Panning Mode
- **Random**: Random stereo positioning
- **Position Based**: Pan based on grain buffer position
- **Spectral Width**: Frequency-dependent panning
- **Expanding**: Progressively wider panning
- **Alternating**: Left-right alternating pattern
- **Alias Generating**: Special panning for aliasing effects

### Buffer Settings
- **Buffer BPM**: Set buffer size based on musical timing (20-300 BPM)
- **Buffer Length**: Choose from 1/16 beat to 64 beats
- **Visualization**: Select visualization mode

### Overrides

#### Index Input Mode
- **Continuous**: Real-time position control
- **Edge-Triggered**: Position updates only on voltage changes

#### Emit Input Mode
- **Trigger**: Create grain on rising edge
- **Stream**: Create grains while signal is high

### Sample Player
- **Sample Mode**: Toggle between live input and sample playback
- **Load Sample**: Browse and load audio files (WAV format)
- **Clear Sample**: Remove loaded sample and return to live mode
- **Current Sample**: Displays filename of currently loaded sample

## Performance Tips

### CPU Optimization
- Reduce grain count for lower CPU usage
- Use "None" visualization mode for minimal overhead
- Lower density settings reduce computational load
- Monitor the Max Grains LED - when constantly lit, consider reducing the COUNT parameter

### Creative Techniques

#### Rhythmic Textures
- Use EMIT input with clock/trigger sources
- Set INDEX input to sequencer for rhythmic position changes
- Adjust SPREAD parameter for rhythmic grain distribution
- Use PATTERN control to select specific distribution algorithms

#### Ambient Soundscapes
- High DENSITY with large SIZE values
- Add FEEDBACK for evolving textures
- Use STEREO parameter for wide, immersive sounds

#### Glitch Effects
- Zero or extremely low JITTER settings for precise, mechanical timing
- Rapid PITCH variations
- Short SIZE with high DENSITY

#### Time-Stretching
- Use WARP parameter for tempo-independent pitch shifting
- Combine with other parameters for complex temporal effects

## Amplitude Compensation

Amplitude compensation (also known as DDAS - Dynamic Density Amplitude Scaling) addresses a fundamental challenge in granular synthesis: as more grains play simultaneously, the overall volume increases dramatically, potentially causing distortion or overwhelming loudness.

### How It Works
When multiple grains overlap, their amplitudes add together. With high grain counts or dense settings, this can result in output levels many times louder than the original input. Amplitude compensation automatically reduces the volume of individual grains based on how many are currently active, using mathematical scaling formulas to maintain more consistent overall output levels.

### Mathematical Implementation
The scaling factor is calculated based on the number of active grains (N) using different power functions:

- **Off**: No scaling applied (scaling factor = 1.0)
- **Low**: Gentle compensation using N^0.25 (fourth root scaling)
- **Medium**: Moderate compensation using N^0.5 (square root scaling)  
- **High**: Aggressive compensation using N^0.75 (three-quarter power scaling)

As more grains become active, the scaling factor decreases, reducing individual grain volume to compensate for the additive effect.

### Settings
Available through the context menu under Grain Management > Amplitude Compensation:

- **Off**: No compensation applied - raw grain output (may be very loud with high grain counts)
- **Low**: Gentle compensation using fourth root scaling - minimal volume reduction
- **Medium**: Balanced compensation using square root scaling - good for most use cases
- **High**: Aggressive compensation using three-quarter power scaling - strong volume reduction for very dense textures

### When to Use
- **Dense textures**: Higher compensation prevents distortion when using many simultaneous grains
- **Live performance**: Helps maintain consistent volume levels when changing grain parameters
- **Recording**: Prevents unwanted clipping and maintains better dynamic range
- **Feedback patches**: Essential when using high feedback settings to prevent runaway amplification

## Polyphonic Outputs

Eight polyphonic outputs provide access to individual grains:
- **Poly 1**: Grains 1-16
- **Poly 2**: Grains 17-32
- **Poly 3**: Grains 33-48
- **Poly 4**: Grains 49-64
- **Poly 5**: Grains 65-80
- **Poly 6**: Grains 81-96
- **Poly 7**: Grains 97-112
- **Poly 8**: Grains 113-128

Each polyphonic output carries up to 16 channels, allowing for detailed grain-level processing in external modules.

## Technical Specifications

- **Sample Rate**: Follows VCV Rack engine (typically 44.1-192 kHz)
- **Bit Depth**: 32-bit floating point internal processing
- **Latency**: Near-zero (real-time processing)
- **Maximum Grains**: 128 (user-configurable)
- **Buffer Size**: Musical timing-based (1/16 beat to 64 beats)
- **Audio Formats**: WAV files for sample mode
- **CV Inputs**: Standard 0-10V range, some with 1V/octave support

## Troubleshooting

### No Audio Output
- Check that audio inputs are connected in live mode (LEFT and/or RIGHT for mono/stereo)
- Verify MIX parameter is not at 0%
- Ensure grain COUNT is above minimum
- Check that DENSITY allows grain creation

### CPU Issues
- Reduce grain count
- Lower density settings
- Disable visualization
- Reduce buffer size

### Grain Pool Full
- Check if Max Grains LED is constantly illuminated
- Reduce COUNT parameter if grain limiting is occurring
- Consider using lower DENSITY settings
- Some schedulers (like Burst Taper) may trigger grain pool limits more frequently

### Volume Issues / Distortion
- **Too loud with dense grains**: Enable or increase Amplitude Compensation (Low/Medium/High)
- **Inconsistent volume levels**: Use Medium or High amplitude compensation
- **Clipping with high grain counts**: Try High amplitude compensation setting
- **Runaway feedback**: Reduce FEEDBACK parameter and enable amplitude compensation

### Unstable Feedback
- Reduce FEEDBACK parameter
- Check for feedback loops in patch
- Lower input gain if using feedback

## Conclusion

Spores offers deep granular synthesis capabilities with extensive real-time control and visualization. Whether used for subtle texture enhancement or radical sound transformation, its flexible parameter set and multiple operating modes make it suitable for a wide range of musical applications.

Experiment with different parameter combinations and take advantage of the CV inputs for dynamic, evolving granular textures that respond to your musical gestures.