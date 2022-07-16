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

#### Copying Step Data

It's possible to copy all step data (including parameter lock settings) from one step to another.  

Here's how:

* Hold down the shift key
* While continuing to holding the shift key, double-click on the blue step button that you wish to copy.
* Don’t lift up the shift yet! Keep it pressed!
* While continuing to hold the shift key, click on the destination step(s). All step data will be copied to the destination steps.
* Finally, release the shift key. This will return the module to normal operation.

#### Shifting Patterns

Sometimes it may be helpful to shift a pattern left or right.  There are two ways to
do this:

In order to shift a single track's data left or right:

1. Hold down the __shift key__
2. Click on one of the blue step buttons
3. Drag left or right

In order to shift all 8 of the tracks' data left or right:

1. Hold down the __shift & control keys__
2. Click on one of the blue step buttons
3. Drag left or right

### Changing Track Start and End

This has changed recently! Changing the sequence start and end is now accomplished by dragging the sequence length indicator from the endpoints.  Click either the start or end location indicated in the screen capture below, then drag left or right.

![GrooveBoxTracks](/docs/images/groovebox/change-start-and-end.jpg)


### Parameter Locks

Parameter locks allow you to adjust playback parameters for each step in a sequence.  Each track has independent parameter locks.

Here's how they work:

1. Select which track you'd like to modify.
2. Select what type of thing you want to modify by using the buttons at the very bottom of the module.
3. Adjust the small knob underneath whichever steps you want to modify.

![GrooveBoxParameterLocks](/docs/images/groovebox/parameter_locks.jpg)

For example, if you wanted to raise the volume on the 5th step of a sequence, select VOLUME, then adjust the 5th knob.

### Memory

There are 16 memory slots in the section labeled "MEM".  Each of these sample slots represent a snapshot of all step buttons, track lengths, and parameter locks. The currently selected memory slot is updated in real-time with any changes that you make.  

A CV input lets you switch between memory slots using a control voltage that ranges from 0 to 10v.  When a cable is inserted into the MEM CV input, the memory buttons are disabled.

#### Copying Memory

Basic Method:
* To copy a memory slot, select the memory slot that you'd with to copy and click the CPY button
* To past a memory slot, select the destination memory slot and click PST

Sneaky Method:
* To copy a memory slot, select the memory slot that you'd with to copy and click the CPY button
* While holding the shift key, click on one or more memory slots to paste the copied memory.  This allows you to copy the currently selected memory slot without having to switch memory slots.

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

#### Master Volume

You can crank up the MIX OUT by using the master volume attenuator.  The master volume does
not affect the individual stereo track outputs.

### Minimal Usage

![GrooveBoxMinimalUsage](/docs/images/groovebox/minimal-usage.jpg)



### Video Tutorial

https://youtu.be/1J5eBhe01sk
