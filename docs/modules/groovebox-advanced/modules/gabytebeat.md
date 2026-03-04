# GABytebeat

A bytebeat audio generator that produces lo-fi, algorithmic audio by evaluating bitwise mathematical equations against an incrementing time counter. It provides nine distinct bytebeat equations, each producing a unique character of harsh, digital sound. Three general-purpose parameters (P1, P2, P3) shape each equation's output, while a rate control sets how fast the time counter advances.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | EQ | Control (CV) | Selects the active equation. 0-10V maps across the 9 equations, added to the base EQ dropdown value. |
| 1 | P1 | Control (CV) | Modulates parameter 1. 0-10V adds 0-128 to the base P1 knob value (clamped to 0-128). |
| 2 | P2 | Control (CV) | Modulates parameter 2. 0-10V adds 0-128 to the base P2 knob value (clamped to 0-128). |
| 3 | P3 | Control (CV) | Modulates parameter 3. 0-10V adds 0-128 to the base P3 knob value (clamped to 0-128). |
| 4 | RATE | Control (CV) | Modulates the clock division rate. 0-10V adds 0-256 to the base RATE knob value (clamped to 1-256). |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 5 | OUT | Audio | Bytebeat audio output scaled to +/-5V. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| EQ | Dropdown | 0-8 (9 options) | 0 (Exploratorium) | Selects which bytebeat equation is evaluated. Options: Exploratorium, Toner, Widerange, LandingGear, Rampcode, Unnamed, Silent, BitWiz, Decoherence. |
| P1 | Knob (integer) | 0 - 255 | 64 | First equation parameter. Used differently by each equation to shape the output. Snaps to integer values. |
| P2 | Knob (integer) | 0 - 255 | 64 | Second equation parameter. Used differently by each equation to shape the output. Snaps to integer values. |
| P3 | Knob (integer) | 0 - 255 | 64 | Third equation parameter. Used differently by each equation to shape the output. Snaps to integer values. |
| RATE | Knob | 0.5 - 8.0 | 2.0 | Controls the clock division rate, determining how many DSP samples pass before the internal time counter `t` increments. Higher values slow the bytebeat down, producing lower-pitched output. |

## Details

### Signal Flow

The module maintains an internal 32-bit unsigned time counter (`t`) that drives all bytebeat equations. On each DSP sample, a clock divider determines whether `t` is incremented. The selected equation is then evaluated using `t` and the three parameters (P1, P2, P3), producing an 8-bit unsigned result (0-255). This result is normalized to a 0.0-1.0 float, remapped to a bipolar -1.0 to +1.0 range, and scaled to +/-5V for the audio output.

### Clock Division (Rate)

The RATE parameter controls a clock divider. The internal counter `clockDivisionCounter` increments every DSP sample and is compared against the clock division value. Only when the counter reaches the division value does `t` advance. This means a clock division of 1 increments `t` every sample (fastest/highest pitch), while a division of 256 increments `t` once every 256 samples (slowest/lowest pitch). The knob range of 0.5-8.0 sets the base value, but CV can push it up to 256.

### CV Modulation

All CV inputs expect unipolar 0-10V signals. Each CV value is scaled and added to the corresponding base parameter value:

- **EQ CV**: Scaled by 0.9 (so 10V spans all 9 equations) and added to the dropdown selection. The result is clamped to 0-8.
- **P1/P2/P3 CV**: Scaled by 12.8 (so 10V adds 128) and added to the knob value. The result is clamped to 0-128 before being passed to the equation.
- **RATE CV**: Scaled by 25.6 (so 10V adds 256) and added to the knob value. The result is clamped to 1-256.

Note that the knob ranges for P1/P2/P3 go up to 255, but the effective value passed to the equations is clamped to 128 (knob value + CV). This means with no CV, knob values above 128 are effectively capped at 128 in the DSP.

### Bytebeat Equations

Each equation uses bitwise operations (XOR, AND, OR, shifts) and arithmetic (modulo, division) on the time counter and parameters. The equations produce characteristic patterns:

- **Exploratorium** (0): Modulo and XOR-based with bit-shifted P3.
- **Toner** (1): Cascaded bit shifts with OR and modulo.
- **Widerange** (2): XOR with subtraction and nested modulo.
- **LandingGear** (3): AND/XOR with feedback from the previous output value (`w`).
- **Rampcode** (4): Multiplicative with nested bit shifts and conditional-like divisions.
- **Unnamed** (5): Dense bit manipulation with magic constant `0xb1a7529`.
- **Silent** (6): Additive with OR operations and division.
- **BitWiz** (7): Subtraction-heavy with large constant and cascaded right shifts.
- **Decoherence** (8): Right shift into division and modulo chains.

Division and modulo operations are safe-guarded against division by zero (returning 0 when the divisor is 0).

## Tips

- Start with the **Exploratorium** equation and sweep P1 and P2 slowly to get a feel for how the parameters reshape the output.
- Send an LFO into the RATE input to create pitch sweeps and time-stretching effects.
- Modulate P1/P2/P3 with slow random or sample-and-hold signals for evolving, generative textures.
- The **LandingGear** equation uses feedback from its own previous output, making it more chaotic and sensitive to parameter changes -- small adjustments can produce dramatic timbral shifts.
- Use the EQ CV input with a sequencer to switch between equations rhythmically, creating beat-synced timbral changes.
- Bytebeat output is inherently harsh and aliased. Pair it with a low-pass filter module to tame the highs, or embrace the grit for industrial and noise patches.
- At very high RATE CV values (pushing the clock division toward 256), the output slows to near-static levels, which can be interesting as a slow modulation source rather than audio.
