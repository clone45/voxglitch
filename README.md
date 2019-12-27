# voxglitch

Modules for VCV Rack.  More info coming soon.

## Repeater

Repeater is a stuttering sample player.  It sounds like if your CD player started skipping.  

### Inputs

Here are the inputs:

* CLK - clock input (very fast clock)
* DIV - clock division. Controls repeat (stutter) length
* POS - sample playback position
* WAV - selects between 5 samples that can be loaded via the context menu
* PITCH - yep, pitch. Can be used to play samples backwards

### Minimal Usage
1. Add a very fast clock signal to the CLK input.
2. Right click on the module to load a .wav sample.  I suggest that the sample be around 2 to 5 seconds long.
2. Tweak the POS and DIV knobs!

![Repeater](/docs/images/repeater-patch-example.png)
