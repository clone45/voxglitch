# voxglitch

Modules for VCV Rack.  More info coming soon.


## Ghosts
![Ghosts](/docs/images/ghosts-front-panel-02.png)
=======
Ghosts is a granular synthesis based sample player.  Load a sample using the right-click context menu and have fun!  In my imagination, this module simulates an old graveyard.  The graveyard spawns "ghosts", and each ghost produces sound.  The sounds that ghosts produce are derived from small slices of your loaded .wav file.  

### Inputs

* SPAWN RATE - How quickly new ghosts rise from their graves.  Spawn rate is counter intuitive: Lower values spawn ghosts more quickly.  This is because SPAWN RATE acts like a kitchen timer.
* QUANTITY - The maximum number of ghosts allowed at one time.  Once there are too many ghosts, the oldest ghosts are returned to their graves.
* LENGTH - The length of the sample slice played by newly spawned ghosts.
* POSITION - In short: An offset into the loaded .wav file.  When a new ghost is spawned, it starts playing back a slice of the loaded .wav file starting at the sample location specified by this input.  

### More Inputs

* PURGE - This is a trigger input that purges all ghosts from the graveyard.
* JITTER - Adds a bit of noise to POSITION.  Helpful in keeping all the ghosts from coalescing to one playback position.
* TRIM - Adjusts output volume.

### Outputs

* WAV - sample playback output

### Minimal Usage

![Ghosts](/docs/images/ghosts-patch-example-01.png)

1. Right click on the module to load a .wav sample. I suggest that the sample be around 3 to 6 seconds long.
2. Switch the Jitter switch on.
3. Tweak the Length, Quantity, Spawn Rate, and Position knobs.

## Repeater
![Repeater](/docs/images/repeater-front-panel-1230224.png)
=======
Repeater is a stuttering sample player.  It sounds like a skipping CD player.  

### Inputs

* CLK - clock input (very fast clock)
* DIV - clock division. Controls repeat (stutter) length
* POS - sample playback position
* WAV - selects between 5 samples that can be loaded via the context menu
* PITCH - yep, pitch. Can be used to play samples backwards

### Outputs

* WAV - sample playback output
* TRG - trigger output.  Fires whenever the sample "stutters"

### Switches and Options

* Smooth - Smooths out pops that would normally occur when setting the sample playback position via the POS input or when switching between .wav files via the WAV input.
* Retrigger (context menu item) - Causes a sample to restart after reaching the end of playback.

### Minimal Usage

![Repeater](/docs/images/repeater-patch-example-1230224.png)

1. Reproduce the patch example shown in the image above.  On the CLOCKED module, notice that the clock ratio for `CLK 1` is set to X16.
2. Right click on the module to load a .wav sample. I suggest that the sample be around 2 to 5 seconds long.
3. Tweak the `POS` and `DIV` knobs!

### Using with a Sequencer

![Repeater](/docs/images/repeater-sequencer-example-1230252.png)


### Please note the following

1. I do intend to update and customize the front panel design eventually.
2. I intend to make a companion x/y controller module similar to: https://youtu.be/1pFrvNx5oAc 5
3. My code is based on Clément Foulc’s PLAY module. A big thank you to him for posting his code. Also, thank you to everyone on these forums for your help!
4. In order to try with this module, you’ll need to compile it (make install) from my source code. Of course, eventually I hope to submit it to the VCV library. It will be free and open source.


## Wav Bank
![WaveBank](/docs/images/wav-bank-front-panel-0101447.png)
=======
The Wav Bank module is a sample player.  It loads all .wav files from a folder and can switch between them on the fly.

### Inputs

* TRG - triggers sample playback
* WAV - selects sample for playback
* Pitch - controls sample playback speed

### Outputs

* WAV - sample playback output

### Example Usage

![WaveBank](/docs/images/wav-bank-sample-patch-0101447.png)

1. Reproduce the patch example shown in the image above.
2. Right click on the module to select a folder containing one or more .wav files.

## XY Controller
![XY](/docs/images/xy-front-panel-01.png)
=======
XY is a CV output generator with built in gesture recording.  Visuals are based on a Reaktor patch whose name eludes me.

### Inputs

* CLK - gesture recording happens at each incoming CLK signal.  Once recording is complete, incoming CLK signals step through the recorded gestures.  Conserve memory and patch size by clocking with as slow a frequency as necessary.
* RESET - Resets playback to the beginning.
* RETRIG - When ON, playback restarts after completing.
* PUNCH - When ON, dragging within the recording area during playback replaces the section of previously recorded gestures with your new gestures.  Great for modifying an existing recording.

### Outputs

* X - Output voltage based on x-axis, ranging from 0 to 10
* Y - Output voltage based on y-axis, ranging from 0 to 10

### Example Usage

![XY Example Usage](/docs/images/xy-sample-patch-01.png)
