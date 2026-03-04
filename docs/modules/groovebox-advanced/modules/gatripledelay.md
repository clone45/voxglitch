# GATripleDelay

A clock-synced triple delay effect with 3D morph crossfading. Three delay lines run simultaneously at musically related clock divisions -- 1/8 note, dotted 1/8 note, and 1/4 note triplet -- and an X/Y morph control smoothly blends between them. The delay times automatically track an incoming clock signal, keeping repeats locked to the tempo. Useful for rhythmic delay textures, "The Edge"-style dotted echoes, polyrhythmic delay patterns, and animated spatial effects.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | IN | Audio | Audio input signal to be delayed and processed by all three delay lines. |
| 1 | CLK | Trigger | Clock input for tempo sync. The module measures the time between rising edges (threshold at 1.0V) to determine the clock period, which sets the base time for all three delay lines. Accepts clock periods between 0.1s and 4.0s (15-600 BPM). |
| 2 | X | Control (CV) | CV modulation for the X morph position. A bipolar +/-5V signal is scaled to +/-1.0 and added to the X knob value. Controls the balance between Delay A and Delay B when Y is low. The effective value is clamped to 0.0-1.0. |
| 3 | Y | Control (CV) | CV modulation for the Y morph position. A bipolar +/-5V signal is scaled to +/-1.0 and added to the Y knob value. Controls how much Delay C is mixed in relative to the A/B blend. The effective value is clamped to 0.0-1.0. |
| 4 | FB | Control (CV) | CV modulation for feedback amount. A bipolar +/-5V signal is scaled to +/-0.5 and added to the FDBK knob value (smaller modulation range than X/Y to prevent runaway feedback). The effective value is clamped to 0.0-0.95. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 5 | OUT | Audio | Mono audio output. The signal is a linear crossfade between the dry input and the morphed wet delay signal, controlled by the MIX parameter. Output is hard-clipped to +/-10V. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| X | Knob | 0.0 - 1.0 | 0.5 | Morph X position. Controls the horizontal position in the crossfade triangle. At 0.0, the A/B balance favors Delay A (1/8 note). At 1.0, it favors Delay B (dotted 1/8). Values in between produce a proportional blend. |
| Y | Knob | 0.0 - 1.0 | 0.0 | Morph Y position. Controls the vertical position in the crossfade triangle. At 0.0, the output is a blend of Delay A and Delay B only (controlled by X). At 1.0, the output is entirely Delay C (1/4 triplet), regardless of the X setting. Values in between smoothly bring Delay C into the mix. |
| FDBK | Knob | 0.0 - 0.95 | 0.4 | Global feedback amount applied equally to all three delay lines. Controls how much of the wet output signal is fed back into the delay buffers along with the input. Higher values produce longer echo trails. The maximum of 0.95 prevents self-oscillation. |
| MIX | Knob | 0.0 - 1.0 | 0.5 | Dry/wet mix. At 0.0 only the dry input is heard; at 1.0 only the blended delay signal is heard. The crossfade is linear: output = input * (1 - MIX) + wet * MIX. |

## Details

### Signal Flow

1. **Clock Sync**: On each rising edge of the CLK input (crossing 1.0V from below), the module measures the elapsed time since the previous clock edge. If the measured period falls within 0.1s to 4.0s (corresponding to roughly 15-600 BPM), it is accepted as the new clock period. Before the first two clock edges are received, the default period is 0.5s (120 BPM). The clock period sets the base time reference for all three delay lines.

2. **Delay Time Calculation**: Each delay line's time is computed as the clock period multiplied by a fixed division ratio:
   - **Delay A**: 0.5x clock period (1/8 note)
   - **Delay B**: 0.75x clock period (dotted 1/8 note)
   - **Delay C**: 0.667x clock period (1/4 note triplet)

   These times are converted to sample counts and clamped to fit within the maximum buffer size of 262144 samples (approximately 5.4 seconds at 48kHz).

3. **CV Modulation**: The X, Y, and FB CV inputs are combined additively with their respective knob values. X and Y CV inputs are scaled from +/-5V to +/-1.0. The FB CV input uses a smaller scaling factor (+/-5V maps to +/-0.5) to keep feedback under control. All effective values are clamped to their valid ranges after modulation.

4. **Crossfade Weight Calculation**: The effective X and Y values are used to compute three crossfade weights using a barycentric-style mapping:
   - Weight C = Y
   - Weight A = (1 - Y) * (1 - X)
   - Weight B = (1 - Y) * X

   The weights are normalized to sum to 1.0. This creates a triangular morph space where corner (0,0) is pure A, corner (1,0) is pure B, and the midpoint (0.5, 1) is pure C.

5. **Weight Smoothing**: The crossfade weights are smoothed using a one-pole lowpass filter (coefficient 0.001) to prevent zipper noise when the X/Y position changes. This produces gradual, artifact-free crossfade transitions.

6. **Interpolated Delay Read**: All three delay lines are read simultaneously using linear interpolation between adjacent buffer samples. This ensures smooth, click-free output even when delay times are not exact integer sample counts, and during clock period changes.

7. **Wet Signal Mixing**: The three delayed signals are mixed according to the smoothed crossfade weights: `wet = delayedA * weightA + delayedB * weightB + delayedC * weightC`.

8. **Feedback Path**: The feedback signal written into all three delay buffers is `input + wet * feedback`. All three buffers receive identical input so the feedback loop is coherent across all delay lines. This shared feedback creates interplay between the three delay times as repeats from one line feed into all three.

9. **Dry/Wet Mix**: The final output is a linear crossfade between the dry input and the wet delay signal: `output = input * (1 - mix) + wet * mix`.

10. **Output Clamping**: The output is hard-clipped to +/-10V to prevent runaway signal levels from high feedback settings.

### Buffer Architecture

The module maintains three independent circular delay buffers, each capable of holding up to 262144 samples. A single shared write position advances through all three buffers simultaneously. The active buffer length is dynamically adjusted to be slightly larger than the longest active delay time (with a 1000-sample safety margin), which keeps memory access patterns efficient. When the module is reset, all three buffers are cleared to silence.

### CV Modulation Behavior

The X and Y CV inputs use direct additive modulation with +/-5V scaled to +/-1.0 of range. This means a +5V signal adds 1.0 to the knob value, and -5V subtracts 1.0. The FB CV input uses a smaller scale (+/-5V maps to +/-0.5) to prevent accidental feedback runaway from hot CV signals. All modulated values are clamped to their valid parameter ranges after the CV offset is applied.

## Tips

- Send a steady clock from a GAClock or GAClockDiv module into the CLK input to keep the delay times musically locked to your patch tempo. The delay times will automatically adapt when the tempo changes.
- For a classic "The Edge" dotted eighth delay sound, set X to 1.0 and Y to 0.0 to isolate Delay B (dotted 1/8). Use moderate feedback (0.4-0.6) and play sparse, rhythmic patterns.
- Modulate the X and Y inputs with slow LFOs to sweep through the morph space over time. This creates evolving delay textures where the rhythmic character shifts between the three delay time ratios.
- Set Y to 1.0 to isolate the 1/4 triplet delay (Delay C), which produces a swinging, off-grid rhythmic feel that works well with straight-time source material.
- Use X at 0.5 and Y at 0.0 to get an equal blend of the 1/8 note and dotted 1/8 delays. This creates a dense polyrhythmic echo pattern where the two delay times interleave.
- Keep feedback below 0.7 for clean, defined repeats. Push it toward 0.9 for washy, ambient buildup where the echoes smear into a reverb-like texture as the three delay lines feed back into each other.
- Set MIX to 1.0 (fully wet) when using GATripleDelay in a parallel effects chain with a GAMixer, so you can control the dry/wet balance at the mixer level.
- Patch a GASequencer or step-driven CV source into the X and Y inputs to switch between different delay characters per step, creating rhythmically animated delay patterns that change with each beat.
- For subtle stereo-like widening in a mono context, use two GATripleDelay modules with slightly different X/Y settings and pan them in a mixer.
