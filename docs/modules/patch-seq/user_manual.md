# PatchSeq User Manual

![PatchSeq](./patchseq-front-panel.png)

## Overview

PatchSeq is a 16-step patch sequencer where each step contains its own complete modular synthesizer patch. Unlike traditional sequencers that sequence notes or CV values, PatchSeq sequences entire sound engines. Each step can have a completely different synthesis architecture, from simple FM tones to complex effect chains.

Think of it as having 16 miniature modular synthesizers that you switch between with each clock pulse. This allows for:

- **Evolving timbres** - Each step can have a distinct sound character
- **Rhythmic variety** - Different synthesis techniques on each beat
- **Sound design exploration** - Quickly compare different patch architectures
- **Performance flexibility** - Crossfading between patches for smooth transitions

### Key Features

- **16 independent patches** - Each step has its own canvas with modules and cables
- **30+ submodules** - Oscillators, filters, envelopes, LFOs, effects, and utilities
- **Visual patch editing** - Drag-and-drop module placement with cable routing
- **External CV integration** - 4 CV inputs routed into patches via Input modules
- **Multiple outputs** - 4 audio outputs for flexible routing
- **Sequencer modes** - Forward, Backward, Pendulum, and Random playback
- **Step states** - Mute or skip individual steps
- **Patch randomization** - Multiple randomization strategies for inspiration
- **Import/Export** - Save and load individual patches as JSON files

### Architecture

PatchSeq consists of two main components:

1. **Host Module** - The outer sequencer with clock input, step buttons, and I/O jacks
2. **Patch Canvas** - The inner editing area where you build synthesis patches for each step

The host module advances through steps based on the external clock. When a step becomes active, its patch is processed and audio is generated. Crossfading between steps ensures click-free transitions.

### Signal Flow

```
External Clock → Step Advancement → Active Patch Processing → Audio Output
       ↓                                    ↑
  CV Inputs ────────────────────────→ Input Modules (in patch)
```

Each patch can access external CV through Input modules, allowing modulation from sources outside the patch. The patch's Output module(s) route audio to the main outputs.

---

## Quick Start

Getting sound from PatchSeq in under a minute:

### Method 1: Randomize (Fastest)

1. **Connect a clock** to the CLK input (any clock module will work)
2. **Connect outputs** - Route OUTPUT 1 to your audio interface or mixer
3. **Click the RAND button** to generate a random patch for the current step
4. **Click RUN** to start the sequencer
5. **Repeat** - Click different step buttons and randomize each one for variety

### Method 2: Build a Simple Patch

1. **Connect clock and outputs** as above
2. **Click a step button** to select which step to edit
3. **Drag a Carrier module** from the browser (right panel) onto the canvas
4. **Drag an Output module** onto the canvas
5. **Connect them** - Click and drag from Carrier's output to Output's input
6. **Click RUN** to hear the tone

### Minimal Patch Example

The simplest patch that produces sound:

```
┌──────────┐     ┌──────────┐
│ Carrier  │────▶│  Output  │────▶ OUTPUT 1
│  (sine)  │     │          │
└──────────┘     └──────────┘
```

### Quick Tips

- **No sound?** Make sure you have an Output module connected in your patch
- **Step buttons** select which patch you're editing (the canvas updates)
- **Red LEDs** above step buttons show the current playback position
- **Press 'R'** while hovering over the module to randomize the current step
- **Shift+Click** a step button to cycle through Normal → Muted → Skip states

---

## Host Module Interface

The host module panel contains all the inputs, outputs, and controls for the sequencer.

### Clock and Reset

#### CLK (Clock Input)

The clock input advances the sequencer to the next step on each rising edge (trigger).

- **Signal type**: Trigger (rising edge detected at ~1V threshold)
- **Purpose**: Steps the sequencer forward through the sequence
- **Rate**: Any clock rate works - the sequencer advances one step per trigger

**Tip**: The clock signal is also available to submodules. Use a **Clock** module inside your patch with its SYNC switch enabled to synchronize internal timing to the external clock.

#### RST (Reset Input)

The reset input returns the sequencer to step 1 and resets the pendulum direction.

- **Signal type**: Trigger (rising edge detected at ~1V threshold)
- **Purpose**: Resets playback position to the beginning
- **Pendulum mode**: Also resets direction to forward

**Tip**: The reset signal is also available to submodules. Use a **Reset** module inside your patch to synchronize LFOs, sequencers, or pattern generators with the host's reset.

### Using Clock and Reset in Patches

Both the external clock and reset signals can be accessed inside your patches:

| Host Signal | Submodule | Usage |
|-------------|-----------|-------|
| CLK input | **Clock** (SYNC mode) | Sync internal clock to external tempo |
| RST input | **Reset** | Trigger resets on LFOs, Sequencers, PatGen |

**Example: Synchronized LFO**
```
┌──────────┐     ┌──────────┐
│  Reset   │────▶│   LFO    │────▶ (to modulation target)
│   (RST)  │     │  (RST)   │
└──────────┘     └──────────┘
```
The LFO resets its phase whenever the host receives a reset trigger.

**Example: Clock-Synced Internal Sequencer**
```
┌──────────┐     ┌──────────┐
│  Clock   │────▶│ Sequencer│────▶ (to pitch or modulation)
│  (SYNC)  │     │  (CLK)   │
└──────────┘     └──────────┘
```
The internal sequencer advances in sync with the host's external clock.

### Mode and Length

#### MODE (Mode CV Input)

The MODE input allows CV control of the sequencer's playback mode.

- **Signal type**: CV (0V to 10V)
- **Purpose**: Selects between the four sequencer modes
- **Behavior**: Overrides the menu-selected mode when a cable is connected

**Voltage Ranges:**

| Voltage | Mode |
|---------|------|
| 0V - 2.5V | Forward |
| 2.5V - 5V | Backward |
| 5V - 7.5V | Pendulum |
| 7.5V - 10V | Random |

**Use Cases:**
- Modulate with an LFO for constantly changing playback direction
- Use a sequencer to create structured mode changes
- Trigger mode changes from external events

**Note**: When no cable is connected, the mode is set via the right-click context menu.

#### LENGTH (Length CV Input)

The LENGTH input modulates the sequence length around the base length set in the menu.

- **Signal type**: CV (-5V to +5V bipolar)
- **Purpose**: Dynamically shorten or extend the active sequence
- **Base length**: Set via right-click menu (1-16 steps)

**Voltage Scaling:**
- **3 steps per volt** - A ±5V range gives ±15 steps of modulation
- **Bipolar** - Negative voltage shortens, positive voltage extends
- **Clamped** - Final length is always between 1 and 16 steps

**Examples:**

| Base Length | CV Input | Effective Length |
|-------------|----------|------------------|
| 8 steps | 0V | 8 steps |
| 8 steps | +2V | 14 steps (8 + 6) |
| 8 steps | -2V | 2 steps (8 - 6) |
| 16 steps | +5V | 16 steps (clamped) |
| 4 steps | -5V | 1 step (clamped) |

**Use Cases:**
- Create evolving sequence lengths with slow LFO modulation
- Dramatic builds by gradually increasing length
- Glitchy effects with fast random modulation
- Live performance control via external CV source

### CV Inputs

#### CV 1-4 (External CV Inputs)

Four general-purpose CV inputs that feed into your patches via the **Input** submodule.

- **Signal type**: CV (±5V, normalized to ±1 internally)
- **Purpose**: Route external modulation sources into your patches
- **Access**: Use an **Input** module inside your patch to access these signals

**Signal Normalization:**
- External CV is divided by 5 before entering patches
- ±5V at the jack becomes ±1 inside the patch
- This matches the internal signal range used by most submodules

**Using CV Inputs in Patches:**

1. Add an **Input** module to your patch canvas
2. The Input module has 4 outputs (1, 2, 3, 4) corresponding to CV 1-4
3. Connect these outputs to any destination in your patch

```
External World          │  Inside Patch
                        │
CV Source ──▶ CV 1 ─────┼──▶ Input (out 1) ──▶ Filter Cutoff
                        │
LFO ──────▶ CV 2 ─────┼──▶ Input (out 2) ──▶ Operator Level
                        │
Sequencer ──▶ CV 3 ─────┼──▶ Input (out 3) ──▶ VCO Pitch
                        │
```

**Use Cases:**
- V/Oct pitch control from external sequencers
- Expression/modulation from controllers
- Envelope followers for audio-reactive patches
- Cross-modulation between VCV modules and PatchSeq

**Tip**: When randomizing patches, PatchSeq automatically creates Input modules and connections for any CV inputs that have cables connected.

#### RAND (Random Trigger Input)

Triggers patch randomization on rising edge.

- **Signal type**: Trigger (rising edge detected)
- **Purpose**: Externally trigger the randomize function
- **Behavior**: Same as clicking the RAND button

**Affected by Rand Button Mode:**
- **Current patch mode**: Randomizes only the currently selected step
- **All patches mode**: Randomizes all 16 steps at once

Set the mode via right-click menu → "Rand Button Mode"

**Use Cases:**
- Automated patch generation on each bar/phrase
- Live performance randomization via trigger sequencer
- Generative composition with timed randomization

### Outputs

#### OUTPUT 1-4 (Audio Outputs)

Four audio outputs carrying the signal from your patch's Output module(s).

- **Signal type**: Audio (±5V nominal range)
- **Purpose**: Main audio outputs from the active patch
- **Soft limiting**: Outputs are soft-limited using tanh saturation to prevent harsh clipping

**Routing Audio to Outputs:**

Inside your patch, use an **Output** module with 4 inputs (1, 2, 3, 4):

```
Inside Patch                    │  External World
                                │
Carrier ──▶ Output (in 1) ──────┼──▶ OUTPUT 1 ──▶ Mixer/Audio
                                │
Effect ───▶ Output (in 2) ──────┼──▶ OUTPUT 2 ──▶ Separate FX Chain
                                │
```

**Multiple Output Modules:**
- You can have multiple Output modules in a single patch
- All Output modules sum together at the main outputs
- Useful for complex patches with parallel signal paths

**Crossfading:**
- When switching between steps, outputs crossfade smoothly (50ms fade)
- This prevents clicks and pops during step transitions
- Both the outgoing and incoming patches play briefly during crossfade

**No Sound Troubleshooting:**
1. Verify you have an **Output** module in your patch
2. Check that something is connected to the Output module's input(s)
3. Ensure the step isn't muted (check step button color)
4. Verify the sequencer is running (RUN light should be lit)

### Buttons

#### RUN Button

Toggles the sequencer between running and stopped states.

- **Running** (light on): Sequencer advances through steps on each clock pulse
- **Stopped** (light off): Sequencer pauses, plays the currently selected editing step

**Critical Behavior Difference:**

| State | What Plays | Clock Response | Step Button Click |
|-------|------------|----------------|-------------------|
| **Running** | Current sequence step | Advances to next step | Selects step for editing (canvas updates) |
| **Stopped** | Selected editing step | Ignored | Selects step AND immediately plays it |

**Stopped Mode as Preview:**

When stopped, PatchSeq acts as a **live preview mode**. Clicking any step button:
1. Selects that step for editing (canvas updates to show its patch)
2. Immediately starts playing that step's audio

This is extremely useful for:
- Auditioning patches without running the full sequence
- Comparing sounds between different steps
- Fine-tuning a patch while hearing it in real-time
- Sound design workflow - edit and hear instantly

**Playback LED Behavior:**
- **Running**: Red LED follows the current playback position in the sequence
- **Stopped**: Red LED shows which step is selected (and playing)

#### RAND Button

Randomizes patches based on the current randomizer settings.

- **Current patch mode**: Randomizes only the currently selected step
- **All patches mode**: Randomizes all 16 steps at once

The button tooltip updates to show the current mode ("Randomize Current Step" or "Randomize All Steps").

**Keyboard Shortcut**: Press **R** while hovering over the module to randomize the current step.

See the [Randomization](#randomization) section for details on randomizer strategies.

---

## Step Buttons and Step States

### Step Buttons

The 16 step buttons along the bottom of the module select which patch you're editing and control step states.

#### Click Behaviors

| Action | Result |
|--------|--------|
| **Click** | Select step for editing (canvas shows that step's patch) |
| **Shift+Click** | Cycle step state: Normal → Muted → Skip → Normal |

#### Visual Indicators

Each step button displays its current state:

| Appearance | State | Meaning |
|------------|-------|---------|
| **Lit (bright)** | Editing | This step is currently selected for editing |
| **Normal (dim)** | Normal | Step plays normally |
| **Muted color** | Muted | Step plays but audio is silenced |
| **Skip color** | Skip | Step is skipped during playback |

**Playback LEDs** (red, above step buttons):
- Show the current playback position when running
- Show the selected editing step when stopped

### Step States

Each step can be in one of three states, cycled via Shift+Click:

#### Normal

The default state. The step plays normally during sequence playback.

#### Muted

The step is **silenced but not skipped**. When the sequencer reaches a muted step:
- The step position advances normally
- No audio is produced (output is silent)
- The step still occupies its time slot in the sequence

**Use Cases:**
- Create rhythmic gaps/rests in your sequence
- Temporarily silence a step while keeping timing intact
- A/B comparisons by muting alternate steps

#### Skip

The step is **completely bypassed**. When the sequencer would land on a skipped step:
- The step is immediately passed over
- Playback continues to the next non-skipped step
- The sequence timing contracts (fewer steps to cycle through)

**Use Cases:**
- Shorten the effective sequence length dynamically
- Create irregular patterns (e.g., skip every 4th step)
- Remove a step without losing its patch contents

**Warning**: If all steps within the sequence length are set to Skip, the sequencer will search through all steps. Avoid skipping every step.

### State Interaction with Stopped Mode

When the sequencer is **stopped** and you click a step:

| Step State | What Happens |
|------------|--------------|
| **Normal** | Step is selected and plays immediately |
| **Muted** | Step is selected, plays silently (no audio) |
| **Skip** | Step is selected, plays immediately (skip only affects running sequence) |

**Note**: Skip state only affects automatic sequence advancement. When manually selecting a step (while stopped), skipped steps still play - they're just bypassed when the clock advances.

---

## Sequencer Modes

PatchSeq offers four playback modes that determine how the sequencer advances through steps. Set the mode via the right-click context menu, or control it dynamically via the MODE CV input.

### Forward Mode

The default mode. Steps play in ascending order, looping back to the beginning.

**Pattern** (8-step example):
```
1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 1 → 2 → ...
```

**Characteristics:**
- Predictable, regular progression
- Natural for melodic sequences
- Most intuitive for traditional step sequencing

### Backward Mode

Steps play in descending order, looping from the beginning to the end.

**Pattern** (8-step example):
```
1 → 8 → 7 → 6 → 5 → 4 → 3 → 2 → 1 → 8 → ...
```

**Characteristics:**
- Creates interesting "reversed" versions of forward sequences
- Useful for creating mirror effects with a second sequencer

### Pendulum Mode

Steps oscillate back and forth between the first and last step, like a pendulum swing.

**Pattern** (8-step example):
```
1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 7 → 6 → 5 → 4 → 3 → 2 → 1 → 2 → ...
```

**Characteristics:**
- End steps play once, middle steps play twice per cycle
- Creates smooth, palindromic progressions
- Full cycle length = (length × 2) - 2 steps
- Great for ambient, evolving patches

**Reset Behavior:**
- When reset is triggered, pendulum direction resets to **forward**
- This ensures deterministic behavior after reset

**Boundary Handling:**
- At step 8: Direction reverses, next step is 7
- At step 1: Direction reverses, next step is 2

### Random Mode

Each clock pulse jumps to a random step within the sequence length.

**Pattern** (unpredictable):
```
1 → 5 → 3 → 8 → 2 → 7 → 4 → 1 → 6 → ...
```

**Characteristics:**
- True random - any step can follow any other step
- Steps can repeat consecutively
- Distribution is uniform across all steps in sequence length
- Great for generative, unpredictable textures

**Note**: Random mode respects the sequence length and skip states. Skipped steps are re-rolled until a valid step is found.

### Mode Selection

#### Via Context Menu

1. Right-click on the module
2. Select "Sequencer Mode"
3. Choose from: Forward, Backward, Pendulum, Random

The selected mode is saved with the patch.

#### Via CV Control

Connect a CV source to the **MODE** input for real-time mode changes:

| Voltage Range | Mode |
|---------------|------|
| 0V - 2.5V | Forward |
| 2.5V - 5V | Backward |
| 5V - 7.5V | Pendulum |
| 7.5V - 10V | Random |

**Use Cases:**
- LFO modulation for constantly shifting playback patterns
- Sequenced mode changes for structured variation
- Random/S&H modulation for unpredictable behavior

**Note**: When the MODE input is connected, it **overrides** the menu-selected mode.

### Mode Interaction with Skip Steps

All modes respect the Skip state. When the sequencer would land on a skipped step:

1. The mode calculates the "natural" next step
2. If that step is set to Skip, it continues in the same direction
3. This repeats until a non-skipped step is found
4. Safety limit: If all steps are skipped, it stops searching after checking all steps

**Example** (Forward mode, steps 3 and 4 skipped):
```
1 → 2 → 5 → 6 → 7 → 8 → 1 → 2 → 5 → ...
       (3,4 skipped)
```

---

## Patch Canvas and Editing

The patch canvas is the central editing area where you build synthesis patches for each step. It displays a visual representation of modules and cables, allowing intuitive drag-and-drop patching.

### Canvas Layout

The PatchSeq interface is divided into:

| Area | Description |
|------|-------------|
| **Host Panel** (left) | I/O jacks, buttons, step controls |
| **Parameter Panel** (middle-left) | Knobs and switches for selected module |
| **Patch Canvas** (center) | Visual module and cable editor |
| **Module Browser** (right) | Scrollable list of available modules |

### Working with Modules

#### Adding Modules

1. Find the desired module in the **Module Browser** (right panel)
2. **Click and drag** the module onto the canvas
3. Release to place the module at that position

The module appears with its default parameters. A placeholder message appears on empty canvases: "Drag modules from browser".

#### Selecting Modules

- **Click** on a module to select it (white border appears)
- The **Parameter Panel** updates to show that module's controls
- Click on empty space to deselect

#### Moving Modules

- **Click and drag** a module to reposition it
- Modules stay within the virtual canvas bounds
- Module positions are saved per-step

#### Duplicating Modules

1. **Right-click** on a module
2. Select **Duplicate** from the context menu
3. A copy appears slightly offset from the original
4. All parameters are copied

#### Deleting Modules

1. **Right-click** on a module
2. Select **Delete** from the context menu
3. All cables connected to that module are also removed

### Working with Cables

Cables connect module outputs to inputs, creating the signal flow of your patch.

#### Creating Cables

1. **Click and drag** from any port (input or output)
2. Drag to the destination port
3. **Release** to complete the connection

**Connection Rules:**
- Cables can only connect outputs to inputs (or vice versa)
- Multiple cables can connect to the same output
- Each input can receive one cable (new cable replaces existing)
- Invalid connections are ignored (no visual feedback)

#### Cable Colors

Cables are color-coded by signal type:

| Color | Signal Type |
|-------|-------------|
| **Blue** | Audio signals |
| **Green** | CV/modulation |
| **Yellow** | Triggers/gates |

Feedback cables (delayed by one sample for loop stability) have a special dashed appearance.

#### Disconnecting Cables

**Method 1 - Via Port Context Menu:**
1. **Right-click** on a port
2. Select **Disconnect Cables**
3. All cables from that port are removed

**Method 2 - Lift and Re-route:**
1. **Double-click** on a port with cables
2. The most recently connected cable "lifts" from that end
3. Drag to a new destination or release to delete

### Viewport Navigation

The patch canvas has a virtual area larger than the visible viewport (1000x600 virtual vs. ~200x150 viewport). Navigate using:

#### Panning

- **Click and drag** on empty canvas space
- The viewport scrolls to reveal different parts of the virtual canvas
- Pan position is saved per-step

#### Auto-Centering

When you switch to a different step (click a step button):
- The viewport automatically centers on that step's module positions
- If modules are spread out, it centers on the bounding box center
- Empty patches center on the default canvas position

### Module Browser

The right-hand panel provides access to all available submodules:

#### Browser Layout

- **Header**: "MODULES" title
- **Scrollable List**: All modules organized by category
- **Scroll Bar**: Drag or scroll wheel to navigate

#### Module Categories

The browser organizes modules roughly by function:

| Category | Modules |
|----------|---------|
| **I/O** | Input, Output |
| **Sound Sources** | Carrier, Modulator, Operator, VCO, Voice, KickDrum, SnareDrum, ByteBeat, Sampler |
| **Modulation** | LFO, Envelope |
| **Processing** | Filter, Mixer, VCA, Atten, Scale |
| **Effects** | Delay, Reverb, Distort, Comb, Pitch, Ring Mod, Wavefolder, Slew |
| **Sequencing** | Sequencer, PatGen, Quantize |
| **Utilities** | Clock, Reset, Constant, Debug |

#### Module Entry

Each browser entry shows:
- **Name**: Module type (e.g., "Carrier", "Filter")
- **Description**: Brief function summary
- **Accent Bar**: Color indicator (orange for I/O, blue-grey for others)
- **Drag Handle**: Three dots (`:::`) indicating draggable

### Per-Step Patches

Each of the 16 steps has its own independent patch:

- **Modules** - Different modules and positions per step
- **Cables** - Independent cable routing per step
- **Parameters** - All knob/switch values per step
- **Pan Position** - Viewport location saved per step

**Editing Workflow:**
1. Click a step button to select it for editing
2. The canvas updates to show that step's patch
3. Make changes (add modules, connect cables, adjust parameters)
4. Changes are saved automatically
5. Click another step to edit a different patch

**Tip**: Use the stopped/preview mode to hear a step while editing it. Click RUN to stop the sequencer, then click any step button to both select and play that step.

---

## Available Submodules

PatchSeq includes over 30 submodules for building complete synthesizer patches. Each module has inputs, outputs, and adjustable parameters shown in the Parameter Panel when selected.

### I/O Modules

#### Input

Routes external CV from the host module's CV 1-4 jacks into your patch.

**Outputs:**
- **1, 2, 3, 4** - Corresponding CV inputs (normalized: ±5V external → ±1 internal)

**Parameters:** None

**Notes:**
- One Input module can provide access to all 4 external CV inputs
- External voltage is divided by 5 for internal use
- When randomizing, Input modules are automatically added for connected external CV jacks

---

#### Output

Routes audio from your patch to the host module's OUTPUT 1-4 jacks.

**Inputs:**
- **1, 2, 3, 4** - Audio to route to corresponding outputs

**Parameters:**
- **LEVEL** (0-1): Master output level

**Notes:**
- Multiple Output modules sum together at the host outputs
- Outputs are soft-limited (tanh saturation) to prevent harsh clipping
- Output is scaled to ±5V for standard VCV Rack levels

---

### Sound Sources

#### Carrier

Primary audio oscillator for FM synthesis. Outputs audio-rate signal.

**Inputs:**
- **FM** - Frequency modulation input (from Modulator or other source)
- **FREQ** - CV control for frequency

**Output:** Audio signal

**Parameters:**
- **FREQ** (8-20000 Hz): Base frequency
- **WAVE** (0-3): Waveform - Sine, Saw, Square, Triangle
- **LEVEL** (0-1): Output level

---

#### Modulator

FM modulator oscillator. Designed to modulate other oscillators.

**Inputs:**
- **FM** - Cascaded FM input
- **FREQ** - CV control for ratio

**Output:** Modulation signal

**Parameters:**
- **RATIO** (0.25-32): Frequency ratio relative to carrier
- **WAVE** (0-3): Waveform
- **DEPTH** (0-4): Modulation depth/amplitude

---

#### Operator

Advanced FM operator with extended features for complex FM synthesis.

**Inputs:**
- **FM** - Frequency modulation input
- **FREQ** - CV control for ratio
- **SYNC** - Hard sync trigger
- **LVL** - Level CV input

**Output:** Audio/modulation signal

**Parameters:**
- **RATIO** (0.25-32): Frequency ratio
- **WAVE** (0-5): Waveform - Sine, Saw, Square, Triangle, Abs(Sine), Half-Sine
- **LEVEL** (0-1): Output level
- **MOD IX** (0-10): Modulation index
- **DETUNE** (-100 to +100 cents): Fine pitch adjustment
- **FDBK** (0-1): Self-feedback amount
- **PHASE** (0-1): Initial phase offset
- **KEYSCL** (0-1): Key scaling amount

**Switches:**
- **MODE**: FM (frequency mod) / PM (phase mod)
- **FREQ**: Ratio (tracks pitch) / Fixed (absolute Hz)

---

#### VCO

Voltage-controlled oscillator with V/Oct pitch input.

**Inputs:**
- **V/O** - V/Oct pitch CV (1V per octave)
- **FM** - Frequency modulation

**Output:** Audio signal

**Parameters:**
- **WAVE** (0-3): Waveform
- **LEVEL** (0-1): Output level

---

#### Voice

Complete synthesizer voice: VCO + ADSR + VCA + Filter in one module.

**Inputs:**
- **GATE** - Gate signal for envelope
- **V/O** - V/Oct pitch input
- **FM** - Frequency modulation

**Output:** Processed audio

**Parameters:**
- **WAVE** (0-3): VCO waveform
- **ATK** (1-500ms): Attack time
- **DEC** (1ms-2s): Decay time
- **SUS** (0-1): Sustain level
- **REL** (1ms-4s): Release time
- **CUT** (0-1): Filter cutoff
- **RES** (0-1): Filter resonance
- **MODE** (0-2): Filter mode - LP, HP, BP
- **E>F** (0-1): Envelope to filter modulation
- **LEVEL** (0-1): Output level

---

#### KickDrum

FM-based kick drum synthesizer.

**Inputs:**
- **TRIG** - Trigger input
- **VEL** - Velocity CV
- **PTCH** - Pitch CV

**Output:** Kick audio

**Parameters:**
- **PITCH** (20-200 Hz): Base pitch
- **CLICK** (0-1): Transient click amount
- **DECAY** (10ms-2s): Decay time
- **PUNCH** (0-1): Punch/attack emphasis

---

#### SnareDrum

Snare drum synthesizer with tonal body and noise components.

**Inputs:**
- **TRIG** - Trigger input
- **VEL** - Velocity CV
- **TONE** - Tone CV

**Output:** Snare audio

**Parameters:**
- **TONE** (100-400 Hz): Tonal component pitch
- **BODY** (0-1): Tonal body level
- **SNAP** (0-1): Attack snap amount
- **NOISE** (0-1): Noise level
- **DECAY** (10ms-1s): Decay time

---

#### ByteBeat

Bytebeat formula-based audio generator for glitchy, algorithmic sounds.

**Inputs:**
- **EQ** - Equation select CV
- **P1, P2, P3** - Parameter CVs
- **RATE** - Sample rate CV

**Output:** Audio signal

**Parameters:**
- **EQ** (0-8): Equation preset - Exploratorium, Toner, Widerange, Landing Gear, Rampcode, Unnamed, Silent Treatment, BitWiz, Decoherence
- **P1, P2, P3** (0-128): Formula parameters
- **RATE** (1-256): Counter increment rate

---

#### Sampler

Sample player with position scrubbing for granular-style playback.

**Inputs:**
- **POS** - Position CV (coarse)
- **FINE** - Position CV (fine)
- **STRT** - Start point CV
- **END** - End point CV

**Outputs:**
- **L** - Left audio output
- **R** - Right audio output

**Parameters:**
- **POS%** (0.01-1): Position input attenuator
- **FINE%** (0.001-1): Fine input attenuator (exponential)
- **START** (0-1): Sample start point
- **END** (0-1): Sample end point

**Context Menu:** Right-click to load sample (.wav, .mp3)

---

### Modulation

#### LFO

Low-frequency oscillator for modulation.

**Inputs:**
- **RATE** - Rate CV
- **RST** - Reset trigger

**Output:** Modulation signal

**Parameters:**
- **RATE** (0.0001-8 Hz): Base rate (exponential control)
- **WAVE** (0-4): Waveform - Sine, Saw, Square, Triangle, Sample & Hold

**Switches:**
- **RANGE**: Low (×1) / High (×20, for audio-rate)
- **OUTPUT**: Uni (0 to +1) / Bi (-1 to +1)

**Notes:** In Low mode, rates as slow as 0.0001 Hz (2.7 hour cycle) are possible.

---

#### Envelope

ADSR envelope generator.

**Inputs:**
- **GATE** - Gate input

**Output:** Envelope CV (0-1)

**Parameters:**
- **ATK** (1ms-2s): Attack time
- **DEC** (1ms-2s): Decay time
- **SUS** (0-1): Sustain level
- **REL** (1ms-4s): Release time

---

### Processing

#### Filter

State-variable filter with multiple modes.

**Inputs:**
- **IN** - Audio input
- **FREQ** - Cutoff frequency CV

**Output:** Filtered audio

**Parameters:**
- **CUTOFF** (0-1): Cutoff frequency
- **RES** (0-1): Resonance
- **MODE** (0-2): Filter type - Lowpass, Highpass, Bandpass

---

#### Mixer

Three-input audio mixer.

**Inputs:**
- **IN 1, IN 2, IN 3** - Audio inputs

**Output:** Mixed audio

**Parameters:**
- **IN 1, IN 2, IN 3** (0-1): Input levels
- **MSTR** (0-1): Master output level

---

#### VCA

Voltage-controlled amplifier.

**Inputs:**
- **IN** - Audio input
- **CV** - Level CV (0-1 range)

**Output:** Amplitude-controlled audio

**Parameters:**
- **LEVEL** (0-1): Base level (multiplied by CV)

---

#### Atten

Attenuverter - scales and inverts signals.

**Inputs:**
- **IN** - Signal input
- **CV** - Attenuation CV

**Output:** Processed signal

**Parameters:**
- **AMT** (-1 to +1): Attenuation amount (-1 = inverted)

---

#### Scale

Scales signal with offset for range adjustment.

**Inputs:**
- **IN** - Signal input
- **CV** - Scale CV

**Output:** Scaled signal

**Parameters:**
- **SCALE** (0-2): Scale multiplier
- **OFFS** (-1 to +1): DC offset

---

### Effects

#### Delay

Simple delay effect with feedback.

**Inputs:**
- **IN** - Audio input
- **TIME** - Delay time CV

**Output:** Delayed audio (wet+dry)

**Parameters:**
- **TIME** (10ms-1s): Delay time
- **FDBK** (0-0.95): Feedback amount
- **MIX** (0-1): Wet/dry mix

---

#### Reverb

Schroeder reverb algorithm.

**Inputs:**
- **IN** - Audio input
- **MIX** - Mix CV

**Output:** Reverb audio

**Parameters:**
- **DECAY** (0.1-0.95): Decay time
- **DAMP** (0-1): High frequency damping
- **MIX** (0-1): Wet/dry mix

---

#### Distort

Waveshaper distortion.

**Inputs:**
- **IN** - Audio input
- **DRV** - Drive CV

**Output:** Distorted audio

**Parameters:**
- **DRIVE** (0-1): Distortion amount

---

#### Comb

Comb filter for metallic/tuned resonance effects.

**Inputs:**
- **IN** - Audio input
- **TIME** - Delay time CV

**Output:** Comb-filtered audio

**Parameters:**
- **TIME** (1-50ms): Delay time (affects pitch)
- **FDBK** (-0.99 to +0.99): Feedback (negative = inverted)

---

#### Pitch

Pitch shifter effect.

**Inputs:**
- **IN** - Audio input
- **PTCH** - Pitch shift CV

**Output:** Pitch-shifted audio

**Parameters:**
- **SHIFT** (-1 to +1): Pitch shift amount

---

#### Ring Mod

Ring modulator - multiplies two signals.

**Inputs:**
- **A** - Audio input A
- **B** - Audio input B

**Output:** A × B

**Parameters:** None

---

#### Wavefolder

West-coast style wavefolder for harmonic enrichment.

**Inputs:**
- **IN** - Audio input
- **FOLD** - Fold amount CV

**Output:** Folded audio

**Parameters:**
- **FOLD** (0-1): Fold amount
- **SYM** (0-1): Symmetry

---

#### Slew

Slew limiter / portamento for smoothing signals.

**Inputs:**
- **IN** - Signal input
- **RATE** - Rate CV

**Output:** Slewed signal

**Parameters:**
- **RISE** (1ms-1s): Rise time
- **FALL** (1ms-1s): Fall time

---

### Sequencing & Utilities

#### Sequencer

8-step CV sequencer.

**Inputs:**
- **CLK** - Clock input
- **RST** - Reset trigger

**Output:** Stepped CV (0-1)

**Parameters:**
- **S1-S8** (0-1): Step values

---

#### PatGen

Pattern generator with pre-defined CV/gate sequences.

**Inputs:**
- **STEP** - Step trigger
- **RST** - Reset trigger
- **PAT** - Pattern select CV
- **LEN** - Length CV

**Outputs:**
- **CV** - Pattern CV output
- **GATE** - Gate output
- **TRIG** - Trigger output

**Parameters:**
- **PAT** (0-127): Pattern number
- **LEN** (1-32): Pattern length

---

#### Quantize

Pitch quantizer with musical scales.

**Inputs:**
- **IN** - Pitch CV input
- **SCL** - Scale select CV

**Output:** Quantized pitch CV

**Parameters:**
- **SCALE** (0-9): Scale - Chromatic, Major, Minor, Pentatonic Major, Pentatonic Minor, Blues, Dorian, Mixolydian, Harmonic Minor, Whole Tone
- **ROOT** (0-11): Root note (C through B)

---

#### Clock

BPM-based clock generator.

**Inputs:**
- **BPM** - BPM CV
- **RST** - Reset trigger

**Output:** Clock trigger

**Parameters:**
- **BPM** (20-300): Tempo
- **DIV** (0-5): Division - 1/1, 1/2, 1/4, 1/8, 1/16, 1/32

**Switches:**
- **CLOCK**: Int (internal) / Sync (follows host clock)

**Notes:** When Sync is enabled, the clock follows the host module's external clock input.

---

#### Reset

Passes through the host module's reset input signal.

**Output:** Reset trigger (mirrors host RST input)

**Parameters:** None

**Notes:** Use to synchronize LFOs, sequencers, or pattern generators with the host's reset.

---

#### Constant

Fixed CV value output.

**Output:** Constant voltage

**Parameters:**
- **VALUE** (-1 to +1): Output value

---

#### Debug

Signal monitor for troubleshooting patches.

**Inputs:**
- **IN** - Signal to monitor

**Output:** Pass-through

**Parameters:** None

**Display:** Shows current input value with 3 decimal places

---

## Parameter Panel

When you select a module on the canvas, the Parameter Panel (left of canvas) displays that module's adjustable controls.

### Knob Controls

- **Drag up/down** to adjust value
- **Double-click** to reset to default
- **Hover** to see current value in header

Knob behavior varies by type:
- **Continuous** (FREQ, LEVEL, etc.): Smooth adjustment
- **Integer** (WAVE, MODE, etc.): Snaps to discrete values
- **Exponential** (RATE, FINE%): More resolution at low values

### Switch Controls

- **Click** to toggle between states
- Current state shown inside the switch

### Value Display

The header shows:
- Module name when no knob is active
- **Knob name: value** when hovering or dragging

### Scrolling

For modules with many parameters (Operator, Voice, etc.):
- **Scroll wheel** to navigate
- **Drag scrollbar** for precise control

---

## Randomization

PatchSeq includes a powerful randomization system that generates complete, playable patches using configurable strategies.

### Using Randomization

#### Quick Randomize

- **Click RAND button** on the panel
- **Press 'R' key** while hovering over the module
- **Send trigger** to the RAND input jack

#### Randomize All Steps

To randomize all 16 steps at once:
1. Right-click → Rand Button Mode → "Randomize all patches"
2. Click the RAND button (or trigger RAND input)

### Randomizer Strategies

Each strategy generates patches with different sonic characteristics. Set via right-click → Randomizer Strategy.

| Strategy | Description |
|----------|-------------|
| **All** (default) | Randomly picks one of the strategies below each time |
| **Aggressive** | Dense, complex patches with many modules and connections |
| **Classic FM** | Traditional FM synthesis - carrier/modulator stacks |
| **Ambient** | Slow-evolving, atmospheric patches with reverb and delay |
| **Experimental** | Unusual routings with feedback, ring mod, wavefolder |
| **Minimal** | Simple, sparse patches with few modules |
| **Pure FM** | FM-only patches using Operators |
| **Subtractive** | VCO + Filter + Envelope architecture |
| **Rhythmic** | Emphasis on clock-driven elements, drums |
| **Kitchen Sink** | Maximum variety - can include any module type |

### Strategy Selection

The "All" setting (default) randomly selects a strategy each time, with weighted probabilities:
- Aggressive: 16%
- Classic FM, Ambient, Experimental: 12% each
- Pure FM, Subtractive, Rhythmic, Kitchen Sink: 10% each
- Minimal: 8%

### External Input Integration

When randomizing, PatchSeq automatically integrates your external CV connections:

1. **Before randomizing**: Connect CV sources to CV 1-4 inputs
2. **Randomize**: The generated patch includes an Input module
3. **Auto-routing**: External inputs are connected to random destinations

This allows external modulation to immediately affect the randomized patch.

---

## Context Menu Options

Right-click on the PatchSeq module to access these options:

### Patches Submenu

Access individual step patches:

For each step (1-16):
- **Export to JSON** - Save the patch as a JSON file
- **Import from JSON** - Load a patch from a JSON file
- **Clear** - Remove all modules and cables from this step

### Behavioral Settings

#### Reset Patches on Switch

- **Enabled** (default): Oscillators/LFOs reset to initial phase when switching steps
- **Disabled**: Continuous playback - phases continue from where they were

Use disabled mode for organic, evolving sounds where precise phase doesn't matter.

### Randomization Section

#### Randomize Current Step

Immediately randomizes the currently selected step.

#### Randomizer Strategy

Select which strategy to use (see Randomization section for details).

#### Rand Button Mode

Controls what the RAND button (and RAND input) affects:
- **Randomize current patch** - Only the selected step
- **Randomize all patches** - All 16 steps at once

### Sequencer Section

#### Sequence Length

Set the base sequence length (1-16 steps). This determines how many steps the sequencer cycles through before looping.

**Note**: This is the base length. The LENGTH CV input can modulate this in real-time.

#### Sequencer Mode

Select the playback mode:
- **Forward** - Steps advance 1, 2, 3... (loops)
- **Backward** - Steps advance 16, 15, 14... (loops)
- **Pendulum** - Steps ping-pong: 1, 2...15, 16, 15...2, 1...
- **Random** - Random step selection each clock

**Note**: When the MODE input has a cable connected, it overrides this menu selection.

### Patch Management

#### Clear Current Step

Removes all modules and cables from the currently selected step.

#### Clear All Steps

Removes all content from all 16 steps. Use with caution!

#### Copy Current Step to...

Copies the current step's patch to another step:
- Select which step to copy to
- Existing content at destination is replaced
- Original step remains unchanged

#### Shift Patches Left/Right

Rotates all patches by 1-15 positions:
- **Shift Left**: Steps rotate left (step 2 becomes step 1, step 1 wraps to step 16)
- **Shift Right**: Steps rotate right (step 1 becomes step 2, step 16 wraps to step 1)

Useful for:
- Reordering sequences without rebuilding
- Creating variations by offsetting timing

---

## Tips and Techniques

### Sound Design Workflow

1. **Stop the sequencer** (click RUN to stop)
2. **Click any step** to select and preview it
3. **Edit while hearing** - changes are immediate
4. **Compare steps** by clicking between them
5. **Start sequencer** when ready to hear full sequence

### Creating Variations

**Method 1: Manual Editing**
1. Build a patch you like
2. Use "Copy current step to..." for other steps
3. Modify each copy slightly

**Method 2: Semi-Random**
1. Set a specific strategy (e.g., "Classic FM")
2. Randomize several steps
3. Keep the ones you like, re-randomize others

### External Modulation

Connect external sources to CV 1-4 for:
- **V/Oct pitch** from sequencers
- **Gates/triggers** for envelopes
- **LFO modulation** for movement
- **Velocity/expression** for dynamics

Use the Input module inside patches to access these signals.

### Performance Tips

- Use **Pendulum mode** for palindromic sequences that don't repeat as obviously
- Set some steps to **Mute** for rhythmic gaps
- Set steps to **Skip** to create irregular time signatures
- Modulate **LENGTH** for evolving pattern lengths
- Modulate **MODE** for changing direction during performance

### Syncing Internal Elements

To sync internal LFOs, sequencers, or pattern generators:
1. Add a **Reset** module to your patch
2. Connect Reset's output to the RST input of LFOs/sequencers
3. Everything resets together when host receives reset trigger

For internal clocks that follow the host:
1. Add a **Clock** module
2. Set CLOCK switch to "Sync"
3. Use Clock's output to drive internal sequencers

---

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **R** | Randomize current step (while hovering over module) |
| **Shift+Click** step button | Cycle step state (Normal → Muted → Skip) |

---

## Troubleshooting

### No Sound

1. Check that you have an **Output** module in your patch
2. Verify something is connected to the Output module's inputs
3. Ensure the step isn't **Muted** (check step button color)
4. Verify the sequencer is **Running** (RUN light should be lit)
5. Check that OUTPUT 1-4 jacks are connected to your audio interface/mixer

### Steps Sound the Same

- Each step has its own independent patch - select different steps to verify
- Steps may have been copied - check each patch's content
- External CV might be dominating the sound - try disconnecting CV inputs

### Clicks Between Steps

- PatchSeq crossfades automatically (50ms fade out, 10ms fade in)
- If you hear clicks, the fade time may be too short for your patch's characteristics
- Consider using patches with their own envelope shaping

### Reset Not Working on LFOs

1. Add a **Reset** module to your patch
2. Connect its output to the LFO's RST input
3. The Reset module outputs the host's reset signal

### External CV Not Affecting Patch

1. Add an **Input** module to your patch
2. Connect the appropriate output (1-4) to your modulation destination
3. Remember: external ±5V becomes ±1 inside the patch
