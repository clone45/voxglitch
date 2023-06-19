## Grain Engine MK2

<img src="images\grain-engine-mk2\grain-engine-mk2-front-panel-v3.png" alt="grain-engine-mk2-front-panel-v2" style="zoom:75%;" />

Grain Engine MK2 is a Granular Synthesis based sample player.  If you're completely new to Granular Synthesis, I would highly recommend that you watch this YouTube video: https://www.youtube.com/watch?v=BWHKKd75V8g.  The first 6 minutes of this video do a wonderful job at introducing the concept of Granular Synthesis.

Unlike my Ghosts module, which is relatively simple and easy to use, Grain Engine is built to be as flexible as possible.  This comes at a cost:  I've sacrificed ease-of-use for flexibility.  But the range of sounds that you can squeeze out of Grain Engine is amazing!  Hopefully this documentation will help you understand what's going on under the hood and guide you with some sagely advice along the way.

Grain Engine MK2 is the next iteration of the older Grain Engine module.

## Quick Start Guide

1. Load a short (3 to 10 second) .wave file by right clicking and selecting sample slot #1
2. Connect the outputs (bottom right ports with the black outline) to an Audio-8 module
3. Ensure that SAMPLE is turned all the way to the left
4. Move the POSITION knob around manually.  You should hear parts of the loaded sample playing.  Once you have played around with it, return the POSITION knob to the far left position.
5. Attach a LFO with a -10/+10 range to the position input
6. Done!  At this point, I would suggest that you explore the different controls and read more about the module below.
7. Please reach out to me and let me know if you are enjoying my modules at clone45@gmail.com. 



## How it works

Grain Engine MK2 requires at least one .wav file loaded into one of the 5 sample slots.  Unlike a traditional sample player, Grain Engine MK2 doesn't play samples forwards or backwards.  It plays "grains", which are very small slices of the sample waveform.  These grains are like bubbles in sparkling water:  They come into being, last for a while, then end.  In Grain Engine, you can control how often the grains are created (RATE), where in the sample they begin playing (POSITION), and a few other attributes, such as pitch and pan. 

### All About Grains

#### Creating a new grain

Grain Engine will continually generate grains by default at a certain rate.  The rate at which grains are automatically created is is controllable via the RATE input.  The higher the value, the more grains are generated per second.

You may override the internal grain generator using the EXT CLK input.  For example, you may connect the square wave output of a VCO to the EXT CLK input.  Providing your own clock input gives you a bit more control and offers you a wider range of values.  The internal grain generator is disabled whenever a cable is connected to the EXT CLK input.

#### Grain Position Illustrated

POSITION is the most important attribute of a grain.  Position is the starting playback position in the .wav file.  Think of it like the dropping the needle of a record player somewhere in the middle of a song.

<img src="images\grain-engine-mk2\1-create-new-grain.png" alt="1-create-new-grain" style="zoom:50%;" />

It's (almost) essential to modulate grain position using LFOs, VCOs, or complex envelope generators.  Other granular synthesis engines do this for you, but I didn't want to make assumptions about how you want to control grain position.  If you don't modulate grain position, all of the grains will start playing back at the same position and sound, at best, like a wavetable playback module.

Modulating the Position in put is the key to success.  You could simply hook an LFO to the Position cv input, but you'll get far more interesting results by mixing together various LFOs and feeding the mixture into the Position input.  Experiment!



#### Jitter

The Jitter input offers a convenient way of introducing some random fluctuations in the overall position.  Applying Jitter makes the sound "lush" with a sense of reverb.  But *beware*!  If you want to create really weird sounds, turn *off* Jitter.

#### Window

The Window is the grain playback length.  A larger window means that the grain will playback a larger portion of the sample.

<img src="images\grain-engine-mk2\grain-window.png" alt="grain-window" style="zoom:50%;" />

#### Pitch

PITCH controls the playback speed of the grains.  After a grain is created, it starts playing at a certain rate.  Pitch controls the playback rate.

#### Rate

The RATE inputs control the rate at which new grains are generated.  In my other modules, it's referred to as Spawn Rate.  RATE inputs are disabled if there's a cable attached to EXT CLK.

#### Grains

The GRAINS inputs set the maximum number of grains allowed to exist at any one time.  Well, sort of.  Because grains are never forcefully removed before they reach the end of their playback window, it's possible to have more grains in memory than requested.   However, I was careful to balance this out so it wouldn't be too CPU intensive.

### Loading and Selecting Samples

#### Loading Samples

To load a sample into Grain Engine MK2, right-click on the front panel and select the sample slot where you'd like to load the sample.  You will be presented with a file selector dialog box.  Currently only .wav files are supported.

<img src="images\grain-engine-mk2\loading-samples.png" alt="loading-samples" style="zoom:20%;" />

#### Selecting Samples

The SAMPLE inputs are used to select which sample is playing.  Empty samples *are* selectable but make no sound.

<img src="images\grain-engine-mk2\sample-inputs.png" alt="sample-inputs" style="zoom:30%;" />



## Managing CPU Utilization

On a fast machine, Grain Engine MK2 should use less than 20% of your CPU.  If you're trying to reduce the CPU usage, avoid using EXT CLK and adjust the RATE input to half or less.  CPU peaks when your grain population grows quickly and dies out slowly.  Rate controls how quickly the grain population grows, so lowering RATE helps limit the population.  

The Grains input, although useful, doesn't put a hard-cap on the population.  Doing so would require that the module kill off grains before they completed playback - which would cause unwanted snaps and pops in the output.



## Tips and Tricks

* For best results, use .wave files between 2 seconds and 10 seconds long.  I've found that complex samples containing a variety of instruments, percussions, and vocals work best.
* For ethereal atmospheres, increase Jitter, Grains, Window, and Rate, and modulate position with a very slow -10/+10 LFO (sawtooth & triangle work best).
* For weirdness, turn off jitter and modulate the position input with complex low frequency oscillators.  Get even weirder by modulating pitch with a VCO.  Maximum your weirdness by using a VCO square wave on the EXT CLK input.
* The JRModules Range LFO or Squinky Labs Functional VCO are great modulation sources for the position input.
