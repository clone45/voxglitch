# Developer notes

## Useful links

- Project status tracker: https://github.com/VCVRack/library/projects/1
- Voxglitch issue for updates: https://github.com/VCVRack/library/issues/626


## Calculations

See the following for a spreadsheet of the panel width and detail layer width for each HP panel width:
https://docs.google.com/spreadsheets/d/1u0FdkN7lIzPHwmSiGGZiQxeL11pMq-TThkF9c5UeU-o/edit?usp=sharing

Constants:

* HP_TO_MM = 5.08 mm
* MM_TO_PX = 2.952756

### Converting panel width from HP to Pixels

#### General equation:

panel_width_in_mm = HP_TO_MM * panel_width_in_hp
panel_width_in_px = MM_TO_PX * panel_width_in_mm

#### Example:

Given a panel width of 28 HP, what is the panel's width in pixels?

panel_width_in_hp = 26 HP
HP_TO_MM = 5.08 mm // constant

Calculate the panel_width_in_px:

  panel_width_in_mm = HP_TO_MM * panel_width_in_hp
  panel_width_in_mm = 5.08 * module_width_in_hp
  panel_width_in_mm = 5.08 * 26
  panel_width_in_mm = 132.08 mm

Convert panel_width_in_mm to pixels

  panel_width_in_px = MM_TO_PX * panel_width_in_mm
  panel_width_in_px = 2.952756 * 132.08 = 390.00 pixels


## Creating detail layers

Detail layer images should be 4x the size of the panel.  You can compute the necessary size like so:

detail_layer_width_px = panel_width_in_px * 4

For example, given a panel of 132.08 px

* detail_layer_width_px = panel_width_in_px * 4
* detail_layer_width_mm = 390.00 pixels * 4
* detail_layer_width_mm = 1560

The HP in the examples is the same HP as Glitch Sequencer. The numbers match those of Glitch Sequencer. 


## Typography

### Input and output labels

Font: Pilat
Weight: Normal
Dark Mode Color (r,g,b): 11, 17, 22
Light Mode Color (r,g,b): 244, 238, 233
Letter spacing: .88
Size: 7pt

### Module name (in rectangle)

Font: Pilat Condensed
Weight: Normal
Dark Mode Color (r,g,b): 11, 17, 22
Light Mode Color (r,g,b): 244, 238, 233
Letter spacing: .60
Size: 6pt

## Positioning

### Medium knobs
When trying to find out the correct y-position of a knob within a scale decal:
knob_position_px = scale_decal_positon_px + 22.78879 px

### Input ring decal
Light mode: 11, 17, 22  65 opacity?

### Medium knob decal
Light mode: 11, 17, 22    65 opacity?



### Attenuator knobs

When trying to find out the correct y-position of an attenuator knob within a scale decal:
knob_position_px = scale_decal_positon_px + 14.73 px

The distance between the scale decal and scale label is 15.0 px

## Attenuator decals
Light mode: 11, 17, 22   65 opacity

### Inputs

The positional difference betwee an input port and the input ( ) indicator is 11.0596 px