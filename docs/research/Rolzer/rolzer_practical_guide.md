# Rollz-5 VCV Rack Module — MVP Specification

## Overview

This document describes an MVP implementation of a Rollz-5 / Rolzer-inspired module for
VCV Rack, developed under the Voxglitch plugin brand. The Rollz-5 is a hardware instrument
by Ciat-Lonbarde built around "roll" circuits — ring oscillators arranged as geometric
polygons — that generate organic, evolving rhythmic patterns through node interconnection.

The goal is a **behaviorally faithful** emulation, not a bit-perfect circuit simulation.
The key musical behaviors to capture are:

- Even rolls produce stable, periodic pulse trains
- Odd rolls produce aperiodic, chaotic pulse timing due to their inherent topological paradox
- Connecting rolls to each other creates complex polyrhythmic and evolving patterns
- Nodes are bidirectional (neither pure input nor pure output)

---

## Core Concept: The Roll Circuit

A "roll" is a ring of N transistor nodes. Each node:

1. Integrates (accumulates) incoming charge toward a threshold
2. When the threshold is crossed, fires an inverted pulse to the next node and resets

### Even vs. Odd Behavior

- **Even rolls (4, 6 nodes):** A pulse traversing the ring arrives back at its origin
  in the same polarity → stable, self-resolving oscillation. The period is determined
  by the capacitance (integration rate) at each node.

- **Odd rolls (3, 5 nodes):** A pulse arrives back inverted → the ring can never resolve.
  This creates an aperiodic, chaotic firing pattern that drifts and evolves over time.
  This behavior emerges naturally from the topology and does NOT require oversampling
  or high-frequency simulation.

### No Oversampling Required

Rather than modeling transistor switching at analog speeds, each node is modeled as a
**continuous-state leaky integrator with threshold-and-fire**. Node state is a float in
[0.0, 1.0]. This approach:

- Runs at VCV Rack's native audio rate (no oversampling needed)
- Naturally produces even-roll periodicity and odd-roll chaos from topology alone
- Is computationally inexpensive

---

## Node Model (Per Node, Per Roll)

```
state += (coupling_input - decay * state) * dt
if state >= threshold:
    fire()         // invert and propagate to next node
    state = 0.0
```

Parameters per node:
- `decay`: controls how fast state bleeds off (affects tempo / roll frequency)
- `threshold`: fire point, nominally 1.0
- `coupling_strength`: how strongly a connected external node influences this node's integration

The `decay` value is derived from the roll's **capacitance setting** (see Tempo Grading below).

---

## MVP Module Design

### Module Name
`Rollz` (or `RollzFive`)

### Roll Geometry
Provide the following rolls, matching the Rolzer's set:

| Roll | Nodes | Behavior |
|------|-------|----------|
| Triangle | 3 | Odd — chaotic |
| Square | 4 | Even — stable |
| Pentagon | 5 | Odd — chaotic |
| Hexagon A | 6 | Even — stable |
| Hexagon B | 6 | Even — stable (second instance for richer patching) |

Five rolls total, matching the Rolzer hardware.

### Tempo Grading (E6 Capacitor System)

Each roll has a **Tempo knob** with 7 positions mapped to the E6 capacitor series,
translated into decay rate values:

| Tempo | Italian | Relative Capacitance | Decay Rate (inverse) |
|-------|---------|----------------------|----------------------|
| 1 | Presto | 1.0µF | Fastest |
| 2 | Allegro | 1.5µF | |
| 3 | Moderato | 2.2µF | |
| 4 | Andante | 3.3µF | |
| 5 | Adagio | 4.7µF | |
| 6 | Lento | 6.8µF | |
| 7 | Grave | 10.0µF | Slowest |

In DSP terms, map these to a `decay` multiplier. Start with a linear mapping and tune
by ear so that Presto produces clearly rhythmic pulses (roughly 4–16 Hz) and Grave
produces very slow pulses (roughly 0.25–1 Hz).

---

## The Sandrode: Bidirectional Nodes

The central design challenge. In the original hardware, nodes are called **sandrodes**
(androgynous nodes) — they are simultaneously input and output. Standard VCV Rack ports
are typed (input or output), which is incompatible with this concept.

### Solution: Internal Patch Matrix + Exposed Ports

Each roll exposes its nodes via **bidirectional connection points** implemented as follows:

1. Each node has an associated **jack port** on the panel
2. Internally, jacks participate in the roll's ring integration directly
3. When two jacks are connected via a patch cable, their node states are **mutually coupled**:
   - Node A's state influences Node B's integration
   - Node B's state influences Node A's integration
   - The coupling is symmetric (same `coupling_strength` in both directions)

### Port Implementation

Since VCV Rack requires ports to be typed, use the following convention:

- Each node jack is an **Output port** that emits the node's current state voltage (0–10V)
- A companion **Input port** sits adjacent, allowing external signals to couple into that node
- Label the output with the node number, label the input with a small arrow or dot
- Visually pair them together on the panel so the user understands they belong to the same node

This preserves VCV Rack's port model while exposing the sandrode concept as clearly as possible.

Alternatively, if the existing Voxglitch internal patching infrastructure supports
bidirectional node types natively, prefer that approach over the paired port workaround.

### Cross-Roll Patching

The primary musical gesture is patching a node from one roll into a node on another roll.
For example: connecting Square node 2 → Pentagon node 4 causes the square's stable pulses
to perturb the pentagon's chaotic timing, creating complex interlocking patterns.

When an external signal is patched into a node's input:
```
state += (coupling_input + external_input * coupling_strength - decay * state) * dt
```

---

## Panel Layout

```
┌─────────────────────────────────────────┐
│  ROLLZ                                  │
│                                         │
│  [TRIANGLE]  [SQUARE]  [PENTAGON]       │
│  Tempo: [knob] Tempo: [knob] Tempo: [knob] │
│  N1 O• I•   N1 O• I•   N1 O• I•        │
│  N2 O• I•   N2 O• I•   N2 O• I•        │
│  N3 O• I•   N3 O• I•   N3 O• I•        │
│             N4 O• I•   N4 O• I•        │
│                         N5 O• I•        │
│                                         │
│  [HEX A]            [HEX B]            │
│  Tempo: [knob]      Tempo: [knob]      │
│  N1 O• I•           N1 O• I•           │
│  N2 O• I•           N2 O• I•           │
│  N3 O• I•           N3 O• I•           │
│  N4 O• I•           N4 O• I•           │
│  N5 O• I•           N5 O• I•           │
│  N6 O• I•           N6 O• I•           │
└─────────────────────────────────────────┘
```

Node outputs emit 0–10V (node state scaled). Node inputs accept 0–10V signals.
Each roll also has one dedicated **Gate Out** jack (orange in the hardware) that fires
a clean 10V gate pulse on each ring cycle completion — useful for triggering envelopes,
Gongs, etc.

---

## MVP Scope (What to Build First)

For the MVP, implement:

1. **All 5 rolls** with their node count and tempo knobs
2. **Node output ports** (one per node per roll, scaled 0–10V)
3. **Node input ports** (one per node per roll, with coupling)
4. **Gate output** per roll (one clean gate pulse per cycle)
5. **Basic panel** — functional but not final aesthetic

Defer to v2:
- Translator circuits (Gongs, Ultrasound Filter, Auto VDog) — these are separate modules
  in the original hardware and can be separate Voxglitch modules later
- Fine-tuning of chaos character vs. tempo interaction
- Visual node state display (LEDs showing current node activity)

---

## File Structure

Follow existing Voxglitch conventions. The new module should be:

- A single `.cpp` / `.hpp` pair for the module logic
- Registered in `plugin.cpp` alongside existing modules
- Panel SVG following existing Voxglitch panel style

---

## DSP Notes

- Run the roll simulation every sample (audio rate) for responsiveness
- Use `float` state variables, not `bool` — continuous state is essential for odd-roll chaos
- Add a small amount of noise (epsilon ~0.001) to each node's integration to prevent
  perfect digital lock-up on even rolls and to seed odd-roll chaos naturally
- Gate outputs should fire a 10V pulse for approximately 1ms (sample-counted) on each
  ring completion
- All node output voltages should be smoothed slightly (one-pole LP at ~1kHz) to avoid
  clicking when used as CV

---

## Reference

- Rollz-5: https://www.ciat-lonbarde.net/ciat-lonbarde/rollz5/index.html
- Rolzer: https://www.ciat-lonbarde.net/ciat-lonbarde/rolzer/index.html
- Key concept: sandrodes, even/odd roll topology, E6 tempo grading