# Impulse - Experimental Physical Modeling Synthesizer User Manual

![Impulse](./impulse_front_panel.jpg)

## Overview

Impulse is an experimental polyphonic physical modeling synthesizer for VCV Rack based on the Karplus-Strong algorithm. It supports up to 16 independent voices and creates plucking sounds based on either noise or bytebeat equations. The module features a comb delay for harmonic complexity, and three distinct excitation modes for diverse sonic possibilities.

[Read about the code architecture](./impulse_architecture.md)

*** CAUTION ***: The comb delay at the final state of this module can cause extemely loud feedback.

## Quick Start Guide

Let's get you up and running with minimum fuss.  To start hearing audio, you'll need to hook up cables to the trigger input, v/oct input, and route the output to an output module.

![Impulse](./minimal_patch.jpg)

The patch above should produce audio, but you might notice that the pitch isn't very interesting.  Here's another basic patch which is worth exploring which uses the Scanner Darkly "Orca's Heart" module to suppoy algorithmicly generated notes and rhythms:

![Impulse](./minimal_patch_2.jpg)


## Input, Excitation, Equation, and Delay

These are the four main panels on the Impulse module.  We'll talk through them one by one.

### Input

![Impulse](./impulse_input_highlighted.jpg)

The two main CV inputs of the Impulse module are trigger input and 1v/Oct input.  These are quite typical for modules, where the trigger input begins note playback, and the 1v/Oct CV input provides the pitch information following the standard 1 volt per octave standard.

#### Polyphony

Impulse is a polyphonic module supporting up to 16 independent voices. The number of active voices is automatically determined by the number of channels in the V/OCT input. When you send a polyphonic cable to the V/OCT input, Impulse will automatically configure itself to match that polyphony, with each voice getting its own pitch from its corresponding channel.

The trigger input also supports polyphony, with each channel triggering its corresponding voice independently. This allows for complex polyphonic patterns where each voice can be triggered at different times.

The audio output automatically matches the polyphony of the V/OCT input, providing independent audio streams for each active voice.

Note that while pitch and triggering are handled per-voice, most synthesis parameters (damping, decay, equation settings, etc.) are shared across all voices. This is controlled via CV channel 0, which applies the same modulation to all active voices.

#### Pitch Sampling

This module is sensitive to the timing between the trigger input and the v/oct pitch input.  If the trigger input arrives before a change in the pitch, the new pitch may not be "sampled".  This can happen when sending the pitch through certain quantizers before it reaches the Impulse module.  In order to help you manage this situation, we've added a "Delayed Pitch Sampling" option in the right-click context menu, which will wait for one clock cycle after a trigger impulse arrives before sampling from the pitch input.


![Impulse](./delayed_pitch_sampling.jpg)

*If you find that you're hearing the wrong note in a sequence, try enabling the "Delayed Pitch Sampling" option in the context menu.*

***

### Excitation

![Impulse](./impulse_excitation_highlighted.jpg)



**Understanding the Excitation Stage**

Before we dive into the Excitation stage, let's cover the basics of Karplus-Strong synthesis.

**The Basic Process**

You start with a short delay buffer filled with an excitation signal (random noise or a bytebeat pattern). Then you loop that buffer over and over. Each time through the loop, two things happen:
- The signal passes through a lowpass filter (controlled by the **damping** parameter)
- It's multiplied by a feedback gain (controlled by the **decay** parameter)

That's the core algorithm.

**What This Creates**

As the loop repeats, high frequencies are progressively filtered out while the fundamental frequency rings out. The fundamental frequency is determined by the delay buffer length.

The **damping** parameter controls the lowpass filter's cutoff frequency. Lower damping removes more high frequencies each cycle, making the tone darker.

The **decay** parameter controls the feedback gain, which determines how long the sound takes to fade away.

**The Result**

Despite being straightforward, this process sounds remarkably like a plucked string—guitar, harp, or similar instruments.

To summarize: delay buffer length sets the pitch (shorter = higher), damping controls brightness, and decay controls sustain.


#### Sources

##### Noise

A noise source is what has traditionally been used in Karplus-Strong.  Note: When the Excitation is set to Noise, the Equation section has no effect.

##### Equation

Instead of using a noise source for the excitation signal, the beginning of a bytebeat equation output can be used.  "Bytebeats" are complex waveforms generated by simple math equations.  The provide a different character to the final output.

##### Equation +

This excitation source is very similar to the Equation source, except that the position where the excitation buffer is sampled from within the bytebeat's output is continually changing.  Think of it like a record player.  For the "Equation" source, only the very beginning section of the record is used.  With "Equation +", the record is continually playing, and the source of the excitation is wherever the playback needle is.

#### Pitch

The Pitch knob sets the base octave of the synthesizer, ranging from -5 octaves to +5 octaves relative to C4. The knob is snap-enabled, meaning it snaps to the nearest octave. You'll typically use this in combination with the V/OCT input when playing melodies.

#### Damping

The Damping control sets the cutoff frequency of a low-pass filter applied in the feedback loop. This is a key control for shaping the timbre.  Lower values (0-30%) produce dark, murky tones with deep resonances. Medium values (40-60%) create a balanced tone that reveals the module's unusual overtone character. Higher values (70-100%) produce bright, metallic tones with emphasized high-frequency artifacts. The damping filter operates in the range of 500Hz to 5000Hz.

#### Decay

Decay determines how long the sound rings out after being triggered, ranging from 0.1 to 10 seconds. Short decay times (0.1-0.5s) create quick plucks and strikes. Medium decay (0.5-2s) produces resonant string-like decay. Long decay times (2-10s) create sustained, drone-like tones with evolving character.

***
 
### Equation

![Impulse](./impulse_equation_highlighted.jpg)

The Equation panel is where things get interesting. This is where you control the bytebeat-based excitation source. Note that these controls only have an effect when the module is in "Equation" or "Equation +" mode (indicated by the second or third LED being lit in the Excitation section).

#### Equation Number Display

The red LED display shows the currently selected equation number (0-40). There are 41 total equations, each with unique timbral and rhythmic characteristics.

#### Equation

The Equation knob selects which bytebeat equation to use. The knob is snap-enabled, so it steps through integer values from 0 to 40.

#### Bleed

Bleed controls how much of the raw bytebeat signal bleeds into the output, ranging from 0% to 50%. At 0%, you hear only the pure Karplus-Strong resonator output. Low values (5-15%) add subtle algorithmic coloration. Medium values (20-35%) create a hybrid physical/algorithmic sound. High values (40-50%) make the output dominated by raw bytebeat texture.

#### Offset

Offset sets the starting position in the bytebeat pattern, ranging from 0 to 352,800 samples. Think of this as jumping to different sections of the equation's evolution.

By default, the offset is only sampled at trigger time. However, you can enable "Continuous Offset Modulation" in the context menu (right-click the module), which allows the offset to be modulated continuously during note sustain, creating evolving, shifting timbres.  Think of Continuous Offset Modulation like manually adjusting the playhead on a tape player.

#### Rate

Rate controls the playback speed of the bytebeat equation, ranging from 0.005x to 8x (or limited by the Max Rate setting in the context menu). Values less than 1.0x slow down the pattern, creating lower-frequency textures. Values greater than 1.0x speed up the pattern, creating higher-frequency textures.

#### Sync Button and LED

The Sync button toggles rate synchronization mode. When the LED is off, the rate is controlled independently by the RATE knob. When the LED is lit, the rate is synchronized to the pitch for harmonic tracking. This creates musical, harmonic relationships between the pitch and equation rate. For example, at 2x rate with sync enabled, the equation runs at twice the fundamental frequency.

#### Injection

Injection sets the amount of additional excitation injected after the initial trigger, ranging from 0x to 10x. At 0x, there's no injection - just a single excitation burst. Low values (1-3x) create extended sustain with gradual re-excitation. High values (5-10x) produce continuous re-excitation and drone-like behavior. 

Technically, this parameter multiplies the initial delay line length to determine how many samples of injection to add.  This helps capture more of a bytebeat's sonic character when in equation mode.

#### P1, P2, P3

These three parameters modify the selected bytebeat equation, with each ranging from 0 to 255. The function of these parameters is equation-specific - each equation uses them differently. The default values are P1=128, P2=64, and P3=32. These are deep sound design controls - experiment with them to discover how each equation responds to these parameters.

***

### Delay

![Impulse](./impulse_delay_highlighted.jpg)

The Delay panel contains the comb delay section, which adds an additional resonant filter for harmonic complexity.  

> Developer note: The purpose of having a delay built into the module is that it syncs to the karplus strong excitation delay, which would be difficult to achieve otherwise.  And it sounds cool.

#### Wet/Dry

The Wet/Dry control sets the mix between the direct signal and the comb-filtered signal, ranging from 0% to 100%. At 0%, you hear the pure Karplus-Strong output (comb bypass). At 50%, you get an equal mix of direct and comb-filtered signal. At 100%, you hear the pure comb-filtered output.

#### Multiplier

The Multiplier scales the comb delay length relative to the fundamental delay line, ranging from 0.25x to 32x.

#### Feedback

Feedback sets the feedback amount for the comb delay, ranging from 0% to 100%.

***

## CV Inputs and Attenuverters

![Impulse](./impulse_attenuators_highlighted.jpg)

Looking at the bottom section of the module, you'll see rows of small knobs and input jacks. Each main parameter (except Pitch and Source) has a dedicated CV input jack and a small attenuverter knob positioned below the main control row.

The attenuverters range from -100% to +100%. At the center position (12 o'clock), there's no CV modulation. Turning clockwise applies positive CV modulation. Turning counter-clockwise applies negative CV modulation, which inverts the CV signal. These allow you to fine-tune the amount and direction of CV control for each parameter.

The CV inputs accept standard 0-10V control voltages, which are normalized to each parameter's range and scaled by the corresponding attenuverter value.

### CV and Polyphony

In polyphonic mode, the CV modulation architecture uses channel 0 for all synthesis parameters (damping, decay, bleed, equation controls, etc.). This means the same modulation is applied to all active voices, keeping the timbral character consistent across the polyphonic output. Only the V/OCT and TRIGGER inputs respond to multiple channels, with each voice tracking its own pitch and trigger state.

***

## Context Menu Options

Right-click the module to access several advanced settings:

#### Delayed Pitch Sampling

When enabled, the module delays pitch sampling by one sample after receiving a trigger. This is useful when using external quantizers, as it allows the quantizer output voltage to settle before Impulse samples the pitch. If you notice pitch instability or wrong notes when using external quantizers, enable this option.

#### Continuous Offset Modulation

When enabled, the OFFSET parameter continuously modulates the bytebeat playback position during note sustain. By default (when disabled), the offset is only set at trigger time. When enabled, offset CV can modulate the playback position in real-time, creating dynamic, shifting timbres. This is particularly useful for evolving drones, animated textures, and LFO-modulated timbral shifts.

#### Clear Comb Delay on Trigger

When enabled (the default), the comb delay buffer is cleared with each new trigger, ensuring clean note starts with no bleed between notes. When disabled, the comb buffer persists across notes, creating overlapping resonances.

#### Max Rate

This sets the maximum value for the RATE knob, constraining its range. Options include 8, 7, 6, 5, 4, 3, 2, 1, 0.5, 0.25, and 0.1. The default is 8. Use this to limit the range for more precise control in specific contexts. For example, if you only need rates up to 2x, set this to 2 for better knob resolution.

***



## Troubleshooting

**No audio output?** Check that triggers are being sent to the trigger input.  Try Noise mode (click SOURCE until the first LED lights) as the simplest mode for testing.

**Wrong pitch or pitch instability?** If you're using a quantizer, enable "Delayed Pitch Sampling" in the context menu. Verify the V/OCT input voltage is in the expected range. Remember that the pitch knob is snap-enabled and may jump to the nearest semitone.

**No sound in Equation modes?** Verify the correct source LED is lit (second or third). Try different equations - some may be very quiet with certain P1/P2/P3 values. Try increasing bleed to hear the raw bytebeat signal. Very slow or fast rates may also produce inaudible results.

**Distortion or clipping?** Reduce the injection parameter if it's set high. Check the bleed level - very high bleed with certain equations can be loud. Reduce comb feedback if it's above 90%. Also check that downstream modules aren't clipping.

***

## Technical Specifications

- **Algorithm**: Karplus-Strong physical modeling with bytebeat excitation options
- **Polyphony**: Up to 16 independent voices
- **Pitch Range**: -5 to +5 octaves (approximately 20Hz to 20kHz depending on sample rate)
- **V/OCT Tracking**: 1V/octave standard, polyphonic
- **Trigger Input**: Polyphonic, independent triggering per voice
- **Audio Output**: ±5V standard VCV Rack audio level, polyphonic
- **CV Inputs**: 0-10V standard range (channel 0 modulates all voices)
- **Equations**: 41 unique bytebeat algorithms

***

## Appendix: Bytebeat Equations

This appendix lists all 41 bytebeat equations available in the Impulse module. Each equation uses the time variable `t` and three parameters (`p1`, `p2`, `p3`) that can be controlled via the P1, P2, and P3 knobs on the module. The default parameter values are P1=128, P2=64, and P3=32.

Note: These equations have been formatted for readability. In the actual implementation, they include safety checks for division by zero and modulo operations.

### Equation 0: Alpha
```
((((t^(p1>>3))-456)*(p2+1))/(mod(t>>(p3>>3), 14)+1))+(t*((182>>(mod(t>>15, 16)))&1))
```

### Equation 1: Omega
```
((((t>>5)|(t<<((p3>>4)+1))>>4)*p1)-(div(t, ((1853>>(mod(t>>(p2>>4), 15))))&1)>>(mod(t>>12, 6))))>>4
```

### Equation 2: Widerange
```
(p1^(t>>(p2>>3)))-(t>>(p3>>2))-mod(t, (t&p2))
```

### Equation 3: Toner
```
(t>>mod(t>>12, (p3>>4)))+(mod((p1|t), p2))<<2
```

### Equation 4: Exploratorium
```
(mod(t, (p1+(mod(t, p2)))))^(t>>(p3>>5)))*2
```

### Equation 5: Melodic
```
(t*((t>>9|t>>p1) & p2)) & (p3+5)
```

### Equation 6: Classic Downward Wiggle
```
((t*9&t>>4)|(t*p1&(t>>(5+(p3>>4))))|(t*3&div(t, p2)))-1
```

### Equation 7: Chewie
```
(p1-(((div(p3+1, t))^p1)|(t^(922+p1))))*div((p3+1), p1)*((t+p2)>>mod(p2, 19))
```

### Equation 8: Buildup
```
((div(t, (p3+1))*t) & mod(t, p1)) & (t - div(t, p2)*632)
```

### Equation 9: Question / Answer
```
((t * ((t>>8) | (t>>p3)) & p2 & t>>8)) ^ ((t & (t>>p1)) | (t>>6))
```

### Equation 10: Sine Wave Wiggle
```
sin((((t >> (p1>>5)) ^ ((mod(t, p2+1)) & ((((t >> 4) ^ (t & 64)) & t) & (mod(t, p2+1))) & (p3>>5))) * 8)*0.1)*127
```

### Equation 11: Rhythmic Cascade
```
((t * ((t >> (p1>>5)) | (t >> 8))) & (mod(t, p2+1))) ^ ((t >> (p3>>5)) * ((t >> 12) & (t >> 10)))
```

### Equation 12: Shifting Rhythm
```
(t >> (p1>>5)) * ((mod(t, p2+2)) & (t >> (p3>>5)))
```

### Equation 13: XOR Pulse
```
(t & p1) ^ ((mod(t, p2+1)) * (t >> (p3>>4)))
```

### Equation 14: Multi-Stream
```
(mod(t, p1+1)) | (mod(t, p2+1)) | (t >> 12) | (t >> (p3>>5)) | (mod(t, (p1+p2)/2+1))
```

### Equation 15: Stepped Melody
```
(seq[mod(t >> (p1>>5), 8)] * ((t >> (p2>>5)) | (t >> 12))) + ((t * (p3>>4)) & (t >> 4))
```
Note: Uses a sequence array: [0, 1, 0, 2, 1, 3, 2, 4]

### Equation 16: Tan Wave
```
tan((t >> (p1>>5))) * (p2+1) + (p3+1)
```

### Equation 17: High Frequency Buzz
```
((t >> (p1>>4)) | (p2>>3)) | (mod(t, p3+1)) * ((t * 3) & (t >> 10))
```

### Equation 18: Fibonacci Chaos
```
(mod(t, p1+1)) | (t >> (p2>>5)) | (t >> 4) | ((((t >> 4) | 16) & scale[mod(t >> 16, 8)]) | (tan((t >> 2)) * 127 + 127) * (fib[mod(t >> 3, 8)] & ((t >> 4) & (mod(t, p3+1)))))
```
Note: Uses Fibonacci sequence [1, 1, 2, 3, 5, 8, 13, 21] and scale array [16, 32, 48, 64, 80, 96, 112, 128]

### Equation 19: Sine Chaos
```
((sin((t >> (p1>>6))) * 127 + 127) | (abs(t >> 10) | ((t & (p2>>4)) * (t >> 4)))) ^ (t >> 16)) | (mod(t, p3+1))
```

### Equation 20: Layered Harmonics
```
(mod(t, p1+1)) | ((t >> (p3>>5)) + (p2>>4)) | ((t >> 8) ^ ((t >> 10) * ((mod(t, p2+1)) + (t >> 4))))
```

### Equation 21: Binary Tree
```
(((t >> (p1>>4)) & (p2>>3)) | ((mod(t, p3+1)) & (t >> 6))) ^ ((powers[mod(t >> 14, 8)] & ((t >> 8) + (t & 34))) | ((t >> 10) & 32) | (mod(t, p1+1)) | (mod(t, p2+1)))
```
Note: Uses powers of 2 array: [1, 3, 7, 15, 31, 63, 127, 255]

### Equation 22: Deep Nested Complexity
```
(mod((t>>p1), mod(t, (p1+p3)+1))) | (mod(t, p2+1)) | ((mod(t, (p3>0 ? p3-1 : 1)+1)*p2))
```

### Equation 23: Shift with Modulo
```
((t>>(p1>>4)) | t | t>>6) * p3 + 4 * ((t & (t>>(p2>>4))) | (t>>(p1>>4)))
```

### Equation 24: Quad Modulo Pattern
```
(((t * div(p1, (mod(t, 10) + 10)) & t>>(p3>>4)) & ((t * (p1>>5)) & t>>(p2>>5))) | (t>>4 & p2))<<3
```

### Equation 25: Masked Complexity
```
(t * ((t>>mod(p1, 16) | p2>>9) & (63-p3) & t>>4)) >> mod(t>>12, 15)
```

### Equation 26: Variable Multiplier Feedback
```
((t * ((t>>8) | (t>>p3)) & p2 & t>>8)) ^ ((t & (t>>p1)) | (t>>6))
```

### Equation 27: Multiplier with Square
```
(t * (p1>>5)) & ((t >> p2) * (t >> p3))
```

### Equation 28: Triple Division Cascade
```
(t * (4 | (7 & t>>13)) >> ((~t>>p1) & 1) & p3) + ((t) * (t>>11 & t>>13) * ((~t>>9) & 3) & p2)
```

### Equation 29: XOR with Variable Multiply
```
((p1 | (t>>mod(t>>13, 14))) * (((t>>mod(p1, 12)) - p2) & 249)) >> mod(t>>13, 6) >> mod(p3>>4, 12)
```

### Equation 30: Combined Multiply-Modulo
```
mod((((t * (p1>>4)) + (t >> 4)) * ((mod(t, p2+1)) >> 4)), p3)
```

### Equation 31: Variable Subtraction
```
(div(mod(mod(t >> mod(p1 >> 12, 12), div(t, mod(p2, 12) + 1)), t >> mod(t >> mod(p3, 10), 12)), mod(t >> mod(p2 >> 2, 15), 15))) << 2
```

### Equation 32: Deep Nested Complexity
```
(((t * 4) - (t >> 4)) >> (t & p1) >> ((((mod(t, p2+1))) ^ ((((mod(t, p3+1))) >> 12) & 0x20)) * (((t >> 10) & 51) + ((t & 40) ^ (t & 23)) + mod((t & 44), (t & 17)))))
```

### Equation 33: Quad Bitwise Mix
```
(div((mod(t>>mod(p1>>12, 12), div(t, (mod(p2, 12) + 1))) - (t>>mod(t>>mod(p3, 10), 12))), mod(t>>mod(p1>>2, 15), 15))) << 4
```

### Equation 34: Modulo Shift Combine
```
((t>>3) * ((p1 - 643) | ((mod(325, max(1, t)) | p2) & t)) - ((t>>6) * 35 / mod(p3, t + 1))) >> 6
```

### Equation 35: Modulo with Product
```
((t * div(50*p1, max(1, mod(t, 20011))) & t>>(p1>>12)) - ((t*3) & t>>mod(p3>>5, 255))) | (t>>4 & (255-p2))
```

### Equation 36: XOR Product Sum
```
((720+((45+p1)|div((t^1>>(t-p1)), p2))*t)-(p1>>(p2>>t)))&(255-p3)
```

### Equation 37: Array Cascade Advanced
```
(1099991*t&t<<(p2-mod(t, p1)))>>(p3>>6)
```

### Equation 38: Large Scale Array
```
(((t * (p1>>5)) + (t >> 10)) >> (freq_array[mod(t >> 4, 48)] & p2))
```
Note: Uses a 48-element frequency array with values from 64 to 967

### Equation 39: Melodic Array Product
```
(melody_array[mod(t * (p1>>5), 20)] * (t >> 4)) * (t >> 10)
```
Note: Uses a 20-element melody array with values from 32 to 456

### Equation 40: Complex Modulo Chain
```
((mod((((t * (p1>>5)) - (t >> 12))), max(1, (t >> 10))) & (mod(t, p2+1))) & (t >> 4))<<(p3>>5)
```

### Understanding the Notation

- `t`: Time variable that increments with each sample
- `p1`, `p2`, `p3`: User-controllable parameters (default: 128, 64, 32)
- `>>` and `<<`: Bit shift operations (right and left)
- `&`, `|`, `^`: Bitwise AND, OR, XOR operations
- `mod(a, b)`: Modulo operation (remainder after division)
- `div(a, b)`: Safe division operation with zero-check
- Trigonometric functions (`sin`, `tan`): Return values normalized to bytebeat range

### Tips for Exploration

1. Each equation responds differently to parameter changes. Start with default values and adjust one parameter at a time.
2. The P1, P2, and P3 parameters have equation-specific effects - there's no universal rule for what they control.
3. Some equations use lookup tables (arrays) that create melodic or rhythmic patterns.
4. Equations with trigonometric functions (10, 16, 18, 19) tend to have smoother characteristics.
5. The Rate and Offset parameters interact with these equations to create evolving textures.
