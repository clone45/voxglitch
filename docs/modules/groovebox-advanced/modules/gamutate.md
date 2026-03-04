# GAMutate

A non-destructive pattern mutation module for step sequencer patterns. It transforms a 64-step velocity pattern in real-time based on a selectable mutation type and an amount parameter. Multiple GAMutate modules can be chained together, with each one applying its effect in sequence. The module is context-aware: when placed in a library patch, it affects only that track's pattern; when placed in the Sequencer Control global patch, it affects all tracks. Global mutations are applied first, followed by per-track mutations.

## Inputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 0 | TYPE | Control (CV) | CV override for the mutation type selection. Accepts 0-10V. The voltage is normalized to [0, 1] and mapped across the 8 mutation types (Shift, Reverse, Thin, Echo, Lit, Dim, Stretch, Fill) with rounding to the nearest type. When this input is connected, it completely overrides the TYPE dropdown parameter. When disconnected, the dropdown selection is used. |
| 1 | AMT | Control (CV) | CV modulation for mutation amount. Accepts 0-10V, which is divided by 10 to produce a 0-1 range that is added to the AMT knob value. The sum is clamped to [0, 1]. This allows external modulation of mutation intensity while the knob sets a base level. |
| 2 | Chain -> | Control (CV) | Chain input for establishing processing order between multiple GAMutate modules. The signal is passed through unchanged to the chain output. Connecting chain ports between GAMutate modules creates a topological dependency that determines the order in which mutations are applied to the pattern. |

## Outputs

| Port | Label | Type | Description |
|------|-------|------|-------------|
| 3 | (unlabeled) | Control (CV) | Chain output. Passes the chain input signal through without modification. Connect this to the Chain input of the next GAMutate module in the chain to establish mutation ordering. |

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| TYPE | Dropdown | Shift, Reverse, Thin, Echo, Lit, Dim, Stretch, Fill | Shift | Selects which mutation algorithm is applied to the velocity pattern. Each type transforms the pattern differently (see Details section). This selection is overridden when the TYPE CV input is connected. |
| AMT | Knob | 0.0 - 1.0 | 0.0 | Controls the intensity of the selected mutation. At 0.0, the pattern is unaffected. At 1.0, the mutation is at full strength. The exact meaning of "amount" depends on the mutation type. This value is summed with the AMT CV input (after voltage-to-unit scaling) and clamped to [0, 1]. |

## Details

### How It Works

GAMutate is a control-rate module -- it does not process or output audio. Instead, the host sequencer reads the module's effective mutation type and amount after DSP processing, then calls the static `applyMutation()` function to transform the 64-step velocity pattern before playback. The mutation operates on a velocity array where values greater than 0 represent active steps (with their velocity level) and 0 represents inactive/silent steps.

Multiple GAMutate modules can be chained together. Each one applies its transformation to the pattern in sequence, with the output of one feeding into the next. The chain input/output ports establish processing order through topological dependency.

### Mutation Types

**Shift** -- Rotates the entire pattern to the left by N steps, where N is determined by the amount. At amount 0.0 there is no shift; at amount 1.0 the pattern shifts by 32 steps (half the 64-step pattern length). Steps that shift past the beginning wrap around to the end. Velocity values are preserved during rotation.

**Reverse** -- Mirrors the pattern around its center. At amount 1.0, the pattern is fully reversed. At fractional amounts, each step has a probability (equal to the amount) of being replaced by its mirrored counterpart, using a deterministic hash function so the result is stable and does not flicker between calls. This creates a gradual morph from the original pattern to its reverse as the amount increases.

**Thin** -- Probabilistically removes active steps from the pattern. Each active step survives with probability (1 - amount). At amount 0.0, no steps are removed. At amount 1.0, all steps are silenced. The removal decision uses a deterministic hash per step, so the same amount always produces the same thinning pattern -- there is no randomness between calls.

**Echo** -- Adds decaying velocity echoes after each active step, inspired by the Quotile Sequencer (Bret Truchan, circa 2009). The algorithm walks left-to-right through the pattern: each active step resets the echo source velocity, and subsequent empty steps receive decaying copies. The decay factor is (amount * 0.8), so at amount 1.0 the decay rate is 0.8 per step. Echoes only fill empty steps and never overwrite existing hits. Echo velocities below 0.01 are zeroed out to prevent inaudible ghost steps.

**Lit** -- Boosts the velocity of all active steps by adding the amount value directly. At amount 0.0, no change occurs. At amount 1.0, all active steps are pushed to full velocity (1.0). Only steps that are already active (velocity > 0) are affected; empty steps remain empty. The result is clamped to [0, 1].

**Dim** -- Reduces the velocity of all active steps by subtracting the amount value directly. At amount 0.0, no change occurs. At amount 1.0, all active steps are silenced (velocity drops to 0). Only steps that are already active are affected. The result is clamped to [0, 1], meaning steps can be completely removed from the pattern at high amounts.

**Stretch** -- Time-expands the pattern by spreading active steps apart. Each active step is repositioned to index `(i * (1 + amount)) % 64`. At amount 0.0, steps remain in place. At amount 1.0, the spacing doubles (every other original position). Steps that collide at the same destination position will overwrite each other (last writer wins). Velocity values are preserved at their new positions. Empty steps remain empty.

**Fill** -- Progressively fills empty steps with new hits as the amount increases. Each empty step has a deterministic threshold in [0, 1). When the amount exceeds a step's threshold, that step activates with a velocity equal to (amount - threshold). This means steps that activate early grow louder as the amount continues to rise. Only empty steps are affected; existing pattern data is never overwritten. At amount 1.0, all previously empty steps will be active.

### CV Modulation Behavior

The TYPE CV input fully overrides the dropdown when connected. The 0-10V range is divided evenly across the 8 mutation types, with rounding to the nearest type. This means approximately 0-0.625V selects Shift, 0.625-1.875V selects Reverse, and so on up to Fill at the top of the range.

The AMT CV input is additive with the knob. For example, if the knob is set to 0.3 and the CV input provides 5V (which maps to 0.5), the effective amount is 0.8. The final value is always clamped to [0, 1].

### Deterministic Hashing

The Reverse, Thin, and Fill mutations use a deterministic hash function to make pseudo-random decisions. The hash takes the step index and a per-mutation seed as inputs and always produces the same output for the same inputs. This means the mutation result is stable across calls -- there is no frame-to-frame jitter or randomness. Changing the amount smoothly morphs the pattern rather than causing it to jump unpredictably.

## Tips

- Use the Shift mutation with a slow LFO on the AMT input to create a pattern that gradually rotates over time, producing evolving rhythmic variations from a single programmed beat.
- Chain a Thin mutation before an Echo mutation. Thinning removes some hits, and Echo fills the gaps with decaying ghost notes, transforming a dense pattern into a more spacious, reverb-like groove.
- The Lit and Dim mutations are useful for dynamic velocity control. Modulate the AMT input with an envelope follower or slow LFO to create crescendos and decrescendos across the pattern's velocity contour.
- Use Fill with a rising ramp CV on the AMT input to create build-ups where empty steps progressively fill in, leading to a fully saturated pattern at the climax.
- The TYPE CV input makes it possible to sequence through different mutation types, creating a pattern that shifts, thins, and fills at different points in a song arrangement.
- Place a GAMutate module in the Sequencer Control global patch to apply the same mutation to all tracks simultaneously. This is useful for global pattern transformations like shifting all tracks together to create a unified rhythmic offset.
- Stretch at moderate amounts (0.3-0.5) can create interesting swing-like effects by displacing steps from their original grid positions.
- Chain multiple GAMutate modules with different types for layered transformations. For example: Shift to rotate the pattern, then Thin to remove some steps, then Echo to add trails. The chain ports ensure they process in the correct order.
