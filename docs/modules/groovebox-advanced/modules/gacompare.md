# GACompare

A voltage comparator module that compares two input voltages and outputs gate signals indicating whether input A is greater than, less than, or equal to input B. It acts as pure combinational logic with no internal state, making it useful for conditional signal routing and event generation.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | A | Control (CV) | First voltage to compare |
| 1 | B | Control (CV) | Second voltage to compare |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | GT | Trigger | Outputs 10V gate when A is greater than B |
| 3 | LT | Trigger | Outputs 10V gate when A is less than B |
| 4 | EQ | Trigger | Outputs 10V gate when A is approximately equal to B (within threshold) |

## Parameters

None.

## Details

GACompare is a stateless comparator that evaluates the difference between its two input voltages every sample. It computes `difference = A - B` and then checks the result against three conditions:

1. If the absolute value of the difference is less than the equality threshold (0.01V), the EQ output goes high (10V) and both GT and LT remain at 0V.
2. If the difference is positive (and above the threshold), the GT output goes high (10V) and the others remain at 0V.
3. If the difference is negative (and beyond the threshold), the LT output goes high (10V) and the others remain at 0V.

The outputs are mutually exclusive -- exactly one of GT, LT, or EQ is high at any given time. The equality threshold of 0.01V prevents rapid toggling between GT and LT when the two inputs are nearly identical, providing a small dead zone for stability.

Because this module has no parameters and no internal state, it responds instantly to input changes with zero latency. It does not have any sync or reset behavior.

## Tips

- Use GACompare with a Sequencer or PatGen module to create conditional triggers. For example, compare a sequencer CV output against a Constant module to fire a trigger only when the sequence value exceeds a threshold.
- Chain multiple GACompare modules to create windowed comparisons (e.g., fire a trigger only when a value is between two bounds by combining the GT output of one comparator with the LT output of another through a Logic AND module).
- Connect the EQ output to detect when two sequencer channels land on the same note value, which can be used to trigger accent patterns or probability gates.
- Use the GT and LT outputs to select between two signal paths via a Switch module, creating voltage-controlled signal routing based on relative levels.
- Pair with an LFO on input A and a Constant on input B to generate a pulse-width-modulated gate signal, where the duty cycle depends on the constant value relative to the LFO waveform.
