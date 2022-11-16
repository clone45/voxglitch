## Voxglitch Groovebox

![GrooveBox](/docs/images/groovebox/groovebox.jpg)

_Graphic Design by [Jim Allman](https://www.linkedin.com/in/jim-allman-399a72/)_

The Voxglitch Groove Box is an 8-track, sample based drum machine with per-step parameter locks.  

### Tracks


The Groove Box has 8 track slots.  Click on a track slot on the LCD screen to select the track for editing.

Tracks are composed of:

* Step button selections
* Assigned samples
* All parameter lock values
* Step start and end positions


### Samples

The Groove Box doesn't ship with built-in sounds.  It's up to the user to provide samples.  Samples should be .wav files, with a 44,100  or higher sample rate for best quality.

#### Loading Samples

There are multiple ways of loading samples:


- Double click on a track's filename to select a replacement sample
- Right-click on a track's filename and select "Load Sample" from the context menu
- Use the module's context-menu to select a folder containing samples (good for setting up preset "kits")

Once a sample is loaded, you can use the up and down arrows displayed to the right of the sample to browse sounds in the same folder.

### Step Buttons

The clackity-clack Step Buttons are used to program the rythm used for playback.

![GrooveBoxStepButtons](/docs/images/groovebox/step_buttons.jpg)

The red led indicators above the step buttons show which step is currently active.

#### Copying Step Data

It's possible to copy all step data (including parameter lock settings) from one step to another.  

Here's how:

![GrooveBoxCopyStep](/docs/images/groovebox/copying_step_data.jpg)


1. Right click on a source step button
2. Select "Copy Step"
3. Right click on a destination step button
4. Select "Paste Step"

At the moment, information can only be copied between steps in the same track.


#### Shifting All Steps Left or Right

Sometimes it may be helpful to shift a pattern left or right.  Here's how:

1. Right click on any step button
2. Select either "Shift Steps Right" or "Shift Steps Left"

#### Randomizing or Clearing All Steps

Here's an easy to to randomize or clear all steps in a track:

1. Right click on any step button
2. Select either Randomize Steps or Clear Steps
3. All steps for the selected track will either be randomized or cleared

### Changing Track Start and End

Changing the sequence start and end is done by dragging the sequence length indicator from the endpoints.  Click either the start or end location as indicated in the screen capture below, then drag left or right.

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
