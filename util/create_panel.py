from svgpanel import *

# Define the panel width and the font to use
PANEL_WIDTH = 20
FONT_FILENAME = 'ShareTechMono-Regular.ttf'
FONT_SIZE = 5

# Create a new panel and set its dimensions
panel = Panel(PANEL_WIDTH)

# Define the colors to use
COLOR_FILL = '#363636'
COLOR_BORDER = '#ffffff'
COLOR_TEXT = '#ffffff'

# Create a gradient for the background
gradient = LinearGradient('gradient', 0, 0, 0, 1, COLOR_FILL, '#262626')
panel.append(gradient)

# Create a border rectangle
border_rect = BorderRect(PANEL_WIDTH, COLOR_FILL, COLOR_BORDER)
panel.append(border_rect)

# Add the title at the top of the panel
title = TextItem('VoxBuilder', Font(FONT_FILENAME), FONT_SIZE)
title_path = title.toPath(0, 10, HorizontalAlignment.Center, VerticalAlignment.Middle)
title_path.setAttrib('fill', COLOR_TEXT)
panel.append(title_path)

# Add the inputs on the left side of the panel
input_pitch = TextItem('Pitch', Font(FONT_FILENAME), FONT_SIZE)
input_pitch_path = input_pitch.toPath(5, 35, HorizontalAlignment.Left, VerticalAlignment.Middle)
input_pitch_path.setAttrib('fill', COLOR_TEXT)
panel.append(input_pitch_path)

input_gate = TextItem('Gate', Font(FONT_FILENAME), FONT_SIZE)
input_gate_path = input_gate.toPath(5, 45, HorizontalAlignment.Left, VerticalAlignment.Middle)
input_gate_path.setAttrib('fill', COLOR_TEXT)
panel.append(input_gate_path)

# Add the params on the right side of the panel
param1 = TextItem('Param1', Font(FONT_FILENAME), FONT_SIZE)
param1_path = param1.toPath(75, 25, HorizontalAlignment.Right, VerticalAlignment.Middle)
param1_path.setAttrib('fill', COLOR_TEXT)
panel.append(param1_path)

param2 = TextItem('Param2', Font(FONT_FILENAME), FONT_SIZE)
param2_path = param2.toPath(75, 35, HorizontalAlignment.Right, VerticalAlignment.Middle)
param2_path.setAttrib('fill', COLOR_TEXT)
panel.append(param2_path)

param3 = TextItem('Param3', Font(FONT_FILENAME), FONT_SIZE)
param3_path = param3.toPath(75, 45, HorizontalAlignment.Right, VerticalAlignment.Middle)
param3_path.setAttrib('fill', COLOR_TEXT)
panel.append(param3_path)

# Save the SVG file
panel.save('VoxBuilder.svg')
