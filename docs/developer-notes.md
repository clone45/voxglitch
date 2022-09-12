# Developer notes

## Creating detail layers

Detail layers should be 4x the size of the panel.  You can compute the necessary size like so:

panel_width_mm * 2.952756 * 4

Here's examples of the XY detail layer calculations
* 101.6 * 2.952756 = 300;  300 * 4 = 1200
* 128.5 * 2.952756 = 379.429;  379.429 * 4 = 1518

Then, to implement the detail layer, add it to the layers section in the theme config file like so:

````
{
  "type": "image",
  "path": "res/xy/themes/default/details.png",
  "width": 101.6,
  "height": 128.5,
  "zoom": 0.25
},
````

## Notes on Digital Sequencer ##

Hi Jeroen!

Yes, my apologies, but when I created the new panels, I made adjustments to how Digital Sequencer stored sequencer values. Previously, values ranged from 0 to 214. If 214 looks odd, it definitely is! It was the height, in pixels, of the drawing area in the top sequencer.

When I wrote Digital Sequencer XP, the drawing area was a different height, but I also took the opportunity to clean up the code a little bit. Instead of storing and saving values with a range from 0 to draw_area_height, I stored the values as floating point numbers between 0 and 1.0 and mapped those values to the draw area height. This meant that I could change the height of the draw area without breaking everyone’s patches, and was something that I should have done in the original Digital Sequencer.

Once Digital Sequencer got a new design, I again took the opportunity to make the same adjustment to how the voltage values were stored, especially because the draw area height changed and was no longer 214.

Unfortunately, that meant storing the values as floating point values instead of integers, which is how they were previously stored. And, yep, it broke everyone’s patches. I knew that a few module updates would break patches, and I’m very sorry about it.

What I can do it revisit Digital Sequencer and see if I can add code that could detect if the values are integer vs float and make the switch automatically without breaking patches. However, that might take a week or so. Would that be helpful?