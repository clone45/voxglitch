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
