## One Zero

One Zero is a gate sequencer that reads sequence data from external text files. Each line in the file represents a different sequence of gates (1s and 0s), and you can switch between sequences using buttons, trigger inputs, or CV control. This makes it perfect for creating dynamic, externally-controlled gate patterns for drums, percussion, and rhythmic modulation.

### Overview

One Zero loads gate sequences from a plain text file where each line contains a series of '1' and '0' characters. A '1' produces a gate output, while a '0' produces no output. This external file approach allows you to create, edit, and organize gate patterns using any text editor, making it easy to design complex rhythmic sequences outside of VCV Rack.

### Inputs

* **STEP** - Clock input to advance the sequencer to the next step
* **RESET** - Resets the sequencer to step 1 (following VCV Rack timing standards with 1ms delay)
* **NEXT** - Trigger input to advance to the next sequence
* **PREV** - Trigger input to go to the previous sequence
* **ZERO** - Trigger input to jump to the first sequence (sequence 0)
* **SEQ** - CV input for sequence selection (-5V to +5V range)

### Outputs

* **GATE** - Gate output (10V when active, 0V when inactive)
* **EOL** - End of Line/Sequence trigger output (fires when sequence wraps around)

### Controls

* **NEXT Button** - Manually advance to the next sequence (LED illuminates when activated)
* **PREV Button** - Manually go to the previous sequence (LED illuminates when activated)
* **ZERO Button** - Manually jump to sequence 0 (LED illuminates when activated)
* **Attenuator Knob** - Attenuates the SEQ CV input (0% to 100%)

### Loading a Sequence File

1. Right-click on the module to open the context menu
2. Select the option to load a file
3. Choose a .txt file containing your sequence data

### File Format

Sequences should be stored in a plain text file with the .txt extension. Each line represents one sequence, and each character should be either '1' or '0'. Unlike One Point, values are NOT separated by commas - they're just sequential characters.

**Example:**

```
1000100010001000
1001001001001001
1111111111111111
1010101010101010
10001000
```

This file contains five sequences:
- Sequence 0: A basic kick drum pattern (16 steps)
- Sequence 1: A sparser kick pattern (16 steps)
- Sequence 2: Constant gates on every step (16 steps)
- Sequence 3: Alternating gates (16 steps)
- Sequence 4: A short pattern (8 steps)

Each sequence can have a different number of steps. Only '1' and '0' characters are recognized - any other characters will be ignored.

### Sequence Selection

There are three ways to select which sequence plays:

1. **Manual Selection**: Use the NEXT, PREV, and ZERO buttons on the module
2. **Trigger Inputs**: Send triggers to the NEXT, PREV, or ZERO inputs
3. **CV Control**: Use the SEQ input with the attenuator knob

The CV input adds or subtracts from the currently selected sequence. The CV range of -5V to +5V is mapped to -20 to +20 sequence offset. Sequence selection wraps around, so if you have 10 sequences and select sequence 12, it will wrap to sequence 2.

### Operation

1. Send clock signals to the STEP input to advance through the sequence
2. When the sequencer encounters a '1', it outputs a gate pulse via the GATE output
3. When the sequencer encounters a '0', the GATE output remains at 0V
4. When the sequence reaches the end, it wraps back to the beginning and sends a trigger out the EOL output
5. Send a trigger to RESET to jump back to the first step of the current sequence
6. Change sequences using the buttons, trigger inputs, or CV input

### Gate Output Behavior

The GATE output produces 10ms pulses (not sustained gates). When a '1' is encountered in the sequence:
- A 10ms pulse at 10V is sent to the GATE output
- This works well with most drum modules and envelope generators
- Multiple consecutive '1's will produce individual pulses for each step

### Reset Behavior

The reset function has special behavior to comply with VCV Rack voltage standards:
- When a reset trigger is received, the sequencer immediately jumps to step 1
- The sequencer ignores incoming STEP triggers for 1 millisecond after reset
- On the first STEP trigger after reset, the sequencer stays at step 1 (doesn't advance)
- Subsequent STEP triggers advance the sequencer normally

This ensures proper synchronization with other modules in your patch.

### Patch Examples

**Basic Drum Pattern**
- Connect a clock source to STEP
- Connect GATE output to a kick drum trigger input
- Load a file with various kick patterns
- Use NEXT/PREV to switch between patterns

**Euclidean-Style Patterns**
- Create text files with Euclidean rhythms (distributed '1's)
- Use the SEQ CV input with an LFO to slowly morph between patterns
- Connect EOL output to trigger fills or pattern changes

**Polyrhythmic Setup**
- Use multiple One Zero modules with the same clock
- Load files with different length sequences (e.g., 16, 12, 7 steps)
- Each module creates its own rhythmic cycle
- Combine outputs for complex polyrhythmic patterns

**Dynamic Pattern Switching**
- Connect a random trigger source to NEXT for unpredictable pattern changes
- Use the EOL output to trigger the NEXT input of another One Zero
- Create evolving, generative rhythm patterns

**Gate to CV Conversion**
- Send GATE output through a sample & hold
- Use a random CV source for melodic variation on rhythmic hits
- Or use GATE to trigger envelope generators for dynamic modulation

### Tips

- Sequences are saved with your patch, so you don't need to reload files each time
- The module remembers which sequence was active when you save
- Each sequence can be a different length - great for polyrhythms
- The EOL output is perfect for triggering fills, switching patterns, or creating accent patterns
- Create libraries of patterns organized by style (techno kicks, hip-hop hats, etc.)
- You can create very long sequences for evolving patterns
- Combine multiple One Zero modules for multi-voice drum programming
- Use the CV input for performance-based pattern morphing
- The gate outputs work great with VCAs for creating rhythmic modulation of any signal

### Creating Pattern Files

You can easily create pattern files using any text editor:

1. **By Hand**: Type out patterns character by character
2. **Algorithmic**: Use scripts (Python, JavaScript, etc.) to generate patterns
3. **Conversion**: Convert drum machine patterns from other formats
4. **Euclidean Generators**: Use online Euclidean rhythm generators and copy the output

Example Python snippet for creating a Euclidean pattern:
```python
def euclidean(steps, pulses):
    pattern = []
    bucket = 0
    for i in range(steps):
        bucket += pulses
        if bucket >= steps:
            bucket -= steps
            pattern.append('1')
        else:
            pattern.append('0')
    return ''.join(pattern)

# Generate some patterns
print(euclidean(16, 4))  # 4 hits in 16 steps
print(euclidean(16, 7))  # 7 hits in 16 steps
```

Save the output to a .txt file and load it into One Zero for instant Euclidean rhythms!
