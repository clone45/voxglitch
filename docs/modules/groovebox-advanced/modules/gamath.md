# GAMath

A utility module that performs basic arithmetic operations on two input signals. It can add, subtract, multiply, or divide signal A by signal B, making it useful for combining CV and audio signals in straightforward mathematical ways.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | A | Audio | First input signal (the left-hand operand for all operations). |
| 1 | B | Audio | Second input signal (the right-hand operand for all operations). |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | OUT | Audio | Result of the selected mathematical operation applied to inputs A and B. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| MODE | Dropdown | Add / Subtract / Multiply / Divide | Add | Selects which arithmetic operation is applied to the two input signals. See the Details section for a description of each mode. |

## Details

### Signal Flow

The module reads two input signals (A and B), applies the selected arithmetic operation, and writes the result to the output. Processing is stateless -- each sample is computed independently with no internal memory or filtering.

### Operations

- **Add**: Outputs A + B. Both signals are summed directly with no scaling. Two 5V signals produce 10V. This is equivalent to a simple signal mixer for two inputs.

- **Subtract**: Outputs A - B. The B signal is subtracted from the A signal with no scaling. This is useful for computing the difference between two signals or for inverting a signal's contribution (patch it to B and leave A unconnected to negate it).

- **Multiply**: Outputs (A * B) * 0.2. The product of the two signals is scaled down by a factor of 5 (multiplied by 0.2) so that two standard +/-5V audio signals produce a +/-5V output rather than +/-25V. This scaling keeps the result within a usable audio voltage range. Functionally, this behaves like a ring modulator -- multiplying two audio-rate signals produces sum and difference frequencies.

- **Divide**: Outputs A / B, clamped to +/-10V. Division by near-zero values is protected: if the absolute value of B is less than or equal to 0.001, the output is set to 0V instead of producing extreme or undefined values. When division does occur, the result is hard-clamped to the +/-10V range to prevent runaway voltages.

### Stateless Processing

GAMath has no internal state, buffers, or filtering. Each sample is computed purely from the current values of A, B, and the selected operation. There is no latency, warmup, or transient behavior.

## Tips

- Use Add mode to mix two signals together. Unlike a dedicated mixer module, there are no level controls, so both inputs contribute at full strength. Pair with attenuverter modules upstream if you need level control before summing.
- Subtract mode is useful for creating sidechain-style ducking effects: patch a steady signal to A and a control signal to B so that when B rises, the output drops.
- Multiply mode with two audio signals produces ring modulation effects -- the output contains sum and difference frequencies of the two inputs. Patch two oscillators in for metallic, bell-like, or inharmonic tones.
- Multiply mode with one audio signal and one CV signal (such as an envelope or LFO) acts as a VCA or amplitude modulator. The 0.2x scaling means a 5V CV signal will pass the audio through at unity gain.
- Use Divide mode cautiously -- it can produce sudden jumps when the B signal crosses near zero. Feed a slow-moving or always-positive CV into B for more predictable results.
- Chain multiple GAMath modules to build more complex mathematical expressions. For example, use one module to multiply two signals and feed its output into a second module's A input to add an offset.
- When using Subtract mode with nothing patched to A (which reads as 0V), the module outputs the negation of B. This is a simple way to invert a signal.
