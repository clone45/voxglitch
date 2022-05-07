## Groovebox

![GrooveBox](/docs/images/groovebox/groovebox.jpg)

The Voxglitch Groove Box is an 8-track, sample based drum machine with per-step parameter locks.  

### Tracks

The Groove Box has 8 track slots.  Each track slot may contain a sample.  Pressing a track button will select it for editing.

Tracks store:

* Step button selections
* Selected sample location on the hard drive.
* All parameter lock values
* Track length

![GrooveBoxTracks](/docs/images/groovebox/tracks.jpg)

### Samples

Groove Box doesn't ship with built-in sounds.  It's up to the user to provide samples.  Samples should be .wav files, with a 44,100  or higher sample rate for best quality.

#### Loading Samples

There are three ways of loading samples:

- Double clicking on a track's filename display screen (preferred way!)
- Using the context-menu (right-click menu) to select individual samples (tedious)
- Using the context-menu to select a folder containing samples (good for setting up preset "kits")

### Step Buttons

The blue Step Buttons are used to program the rythm used for playback -- just like in a typical drum machine.

![GrooveBoxStepButtons](/docs/images/groovebox/step_buttons.jpg)

The red led indicators above the step buttons show which step is currently active.

### Changing Track Length

You can change the length of a track by shift-clicking on a step button.

### Parameter Locks

Parameter locks allow you to adjust playback parameters for each step in a sequence.  Each track has independent parameter locks.

Here's how they work:

1. Select which track you'd like to modify.
2. Select what type of thing you want to modify by using the buttons at the very bottom of the module.  Current options are Volume, Pan, Pitch, Ratcht, Offset, Probability Percentage, Loop, and Reverse
3. Adjust the small knob underneat whichever steps you want to modify.

![GrooveBoxParameterLocks](/docs/images/groovebox/parameter_locks.jpg)

For example, if you wanted to raise the volume on the 5th step of a sequence, select VOLUME, then adjust the 5th knob.

### Memory

There are 16 memory slots in the section labeled "MEM".  Each of these sample slots represent a snapshot of all step buttons, track lengths, and parameter locks. The currently selected memory slot is updated in real-time with any changes that you make.  

- To copy a memory slot, select the memory slot that you'd with to copy and click CPY
- To past a memory slot, select the destination memory slot and click PST

A CV input lets you switch between memory slots using a control voltage that ranges from 0 to 10v.  When a cable is inserted into the MEM CV input, the memory buttons are disabled.

### Clock and Reset

#### Clock

It's necessary to provide a clock signal at the CLK (x32) input.  This steps forward playback of the sequencer.  

!!!   **IMPORTANT**  !!!

A very fast clock signal is required for typical playback.  This is due to the ratcheting feature, which requires precise timing.  I recommend using the Clocked module from Impromptu and setting the RATIO to x32.

#### Reset

The reset input is used to reset playback to the beginning.

### Outputs

Individual sample outputs are available at the top of the module  A stereo mix output is available at the two output jacks at the top-right of the module.  

![GrooveBoxParameterLocks](/docs/images/groovebox/outputs.jpg)

### Minimal Usage

![GrooveBoxMinimalUsage](/docs/images/groovebox/minimal-usage.jpg)



### Video Tutorial

https://youtu.be/1J5eBhe01sk
