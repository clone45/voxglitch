# voxglitch

Modules for VCV Rack.  More info coming soon.

## Repeater


![Repeater](/docs/images/repeater-front-panel-1230224.png)
=======
Repeater is a stuttering sample player.  It sounds like a skipping CD player.  

### Inputs

Here are the inputs:

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

1. Reproduce the patch example that I posted in the image above.  On the CLOCKED module, notice that the clock ratio for CLK 1 is set to X16.
2. Right click on the module to load a .wav sample. I suggest that the sample be around 2 to 5 seconds long.
3. Tweak the POS and DIV knobs!

### Using with a Sequencer

![Repeater](/docs/images/repeater-sequencer-example-1230252.png)


### Please note the following

1. I do intend to update and customize the front panel design eventually.
2. I intend to make a companion x/y controller module similar to: https://youtu.be/1pFrvNx5oAc 5
3. My code is based on Clément Foulc’s PLAY module. A big thank you to him for posting his code. Also, thank you to everyone on these forums for your help!
4. In order to try with this module, you’ll need to compile it (make install) from my source code. Of course, eventually I hope to submit it to the VCV library. It will be free and open source.
