# GALogic

A combinational logic gates module that takes two gate/trigger inputs and produces four simultaneous logic outputs: AND, OR, XOR, and NOT A. It provides fundamental Boolean operations for combining and transforming gate and trigger signals within a patch.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | A | Trigger | First gate/trigger input; interpreted as high when voltage exceeds 1V |
| 1 | B | Trigger | Second gate/trigger input; interpreted as high when voltage exceeds 1V |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 2 | AND | Trigger | Outputs 10V when both A and B are high; 0V otherwise |
| 3 | OR | Trigger | Outputs 10V when either A or B (or both) are high; 0V otherwise |
| 4 | XOR | Trigger | Outputs 10V when exactly one of A or B is high; 0V when both are the same |
| 5 | !A | Trigger | Outputs 10V when A is low; 0V when A is high (inverts input A only) |

## Parameters

None.

## Details

GALogic is a stateless, pure combinational logic module. Each sample, it reads the voltages on inputs A and B and converts them to Boolean values using a threshold of 1V -- any voltage above 1V is treated as true/high, and anything at or below 1V is treated as false/low. It then computes four standard Boolean operations simultaneously:

1. **AND**: True only when both A and B are high. This is useful for gating one trigger stream with another, ensuring events fire only when two conditions are met at the same time.
2. **OR**: True when at least one of A or B is high. This merges two trigger streams so that an event from either source produces output.
3. **XOR** (exclusive or): True when exactly one input is high but not both. This detects disagreement between two gate signals and suppresses output when both fire simultaneously.
4. **!A** (NOT A): The logical inverse of input A. This output is high whenever A is low, and low whenever A is high. Input B has no effect on this output.

All outputs produce 0V for false and 10V for true, which are standard gate/trigger levels in the GrooveboxAdvanced environment. Because the module has no internal state, parameters, or memory, it responds instantly to input changes with zero latency. There is no hysteresis on the input threshold.

The module has no reset or sync behavior. It occupies a standard-width tile with a 1.5x height multiplier to accommodate its six ports (two inputs and four outputs).

## Tips

- Use the AND output to create coincidence detection: connect two different trigger sources (e.g., two PatGen modules or clock dividers) and the AND output fires only when both triggers arrive on the same sample. This is useful for creating rhythmic accents at points where two patterns overlap.
- Use the OR output to merge trigger streams from multiple sources into a single stream. For example, combine the outputs of two Clock Divider modules to create a composite rhythm that includes beats from both divisions.
- Use the XOR output to create fills or variations: when two regular clock divisions are XOR'd together, the output fires on beats where only one division is active, naturally creating syncopated patterns that avoid the strong beats where both clocks coincide.
- Use the !A output to invert a gate signal for alternating behavior. For instance, connect a clock to input A, then use the AND output (with another signal on B) and the !A output to route triggers to two different sound sources on alternating clock phases.
- Chain multiple GALogic modules together for more complex Boolean expressions. For example, take the AND output of one GALogic and the OR output of another and feed them into a second GALogic to build three-input logic functions.
- Pair with a GACompare module: feed the GT or LT output of a comparator into one input and a clock into the other. The AND output will only pass clock triggers while the comparison condition is true, creating conditionally gated rhythms.
