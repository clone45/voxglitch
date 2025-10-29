## One Point

One Point is a CV sequencer that reads sequence data from external text files. Each line in the file represents a different sequence, and you can switch between sequences using buttons, trigger inputs, or CV control. This makes it ideal for creating dynamic, externally-controlled CV patterns.

### Overview

One Point loads sequences from a comma-separated text file where each line contains a series of floating-point values. This external file approach allows you to create, edit, and organize sequences using any text editor, and the module will remember which file and sequence you were using when you save your patch.

### Inputs

* **STEP** - Clock input to advance the sequencer to the next step
* **RESET** - Resets the sequencer to step 1 (following VCV Rack timing standards with 1ms delay)
* **NEXT** - Trigger input to advance to the next sequence
* **PREV** - Trigger input to go to the previous sequence
* **ZERO** - Trigger input to jump to the first sequence (sequence 0)
* **SEQ** - CV input for sequence selection (-5V to +5V range)

### Outputs

* **CV** - CV output of the current step value
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

Sequences should be stored in a plain text file with the .txt extension. Each line represents one sequence, and values within a sequence are separated by commas.

**Example:**

```
0.0,1.0,2.0,3.0,4.0,5.0
-5.0,-3.0,-1.0,1.0,3.0,5.0
0.5,0.5,0.5,0.5,0.5,0.5
1.0,0.0,-1.0,-2.0,-3.0,-4.0
```

This file contains four sequences:
- Sequence 0: Ascending values from 0V to 5V
- Sequence 1: Ascending values from -5V to 5V
- Sequence 2: Constant 0.5V
- Sequence 3: Descending values from 1V to -4V

Each sequence can have a different number of steps. Values can be any floating-point number and will be output directly as voltages.

### Sequence Selection

There are three ways to select which sequence plays:

1. **Manual Selection**: Use the NEXT, PREV, and ZERO buttons on the module
2. **Trigger Inputs**: Send triggers to the NEXT, PREV, or ZERO inputs
3. **CV Control**: Use the SEQ input with the attenuator knob

The CV input adds or subtracts from the currently selected sequence. The CV range of -5V to +5V is mapped to -20 to +20 sequence offset. Sequence selection wraps around, so if you have 10 sequences and select sequence 12, it will wrap to sequence 2.

### Operation

1. Send clock signals to the STEP input to advance through the sequence
2. The module outputs the voltage value at the current step via the CV output
3. When the sequence reaches the end, it wraps back to the beginning and sends a trigger out the EOL output
4. Send a trigger to RESET to jump back to the first step of the current sequence
5. Change sequences using the buttons, trigger inputs, or CV input

### Reset Behavior

The reset function has special behavior to comply with VCV Rack voltage standards:
- When a reset trigger is received, the sequencer immediately jumps to step 1
- The sequencer ignores incoming STEP triggers for 1 millisecond after reset
- On the first STEP trigger after reset, the sequencer stays at step 1 (doesn't advance)
- Subsequent STEP triggers advance the sequencer normally

This ensures proper synchronization with other modules in your patch.

### Patch Examples

**Basic Usage**
- Connect a clock source (like CLKD from Impromptu Modular) to STEP
- Connect CV output to an oscillator's V/OCT input
- Load a file with melodic sequences

**Dynamic Sequence Switching**
- Use a random trigger source connected to NEXT to randomly progress through sequences
- Connect an LFO to the SEQ input for slow, evolving sequence changes
- Use the EOL output to trigger sequence changes or other events in your patch

**Polyphonic Patches**
- Use multiple One Point modules with the same clock source
- Load different sequence files for harmony/counterpoint
- Chain the EOL outputs to create polymetric patterns

### Tips

- Sequences are saved with your patch, so you don't need to reload files each time
- The module remembers which sequence was active when you save
- Use the EOL output creatively - it can trigger envelope generators, switch sequences, or create rhythmic accents
- Negative CV values work great for modulation and don't need to be quantized to notes
- Each sequence can be a different length, creating interesting polyrhythmic patterns
