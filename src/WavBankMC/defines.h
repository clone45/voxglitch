#define GAIN 5.0

// There's not many times that you would want smoothing disabled, so to make
// room on the front panel for the loop switch, I'm simply going to set
// smoothing to true.  If there's demand, I'll add an option to the right-click
// context menu for enabling and disabling smoothing.

// #define SMOOTH_ENABLED 1

#define NUMBER_OF_CHANNELS 16

#define NUMBER_OF_SAMPLE_DISPLAY_ROWS 21

#define READOUT_WIDTH 110
#define READOUT_HEIGHT 360


// Constants for trig_input_response_mode
#define TRIGGER 0
#define GATE 1

#define RESTART_PLAYBACK 0
#define CONTINUAL_PLAYBACK 1

// Column and row grid system
#define COL0 0
#define COL1 3.175
#define COL2 6.35
#define COL3 9.525
#define COL4 12.7
#define COL5 15.875
#define COL6 19.05
#define COL7 22.225
#define COL8 25.4
#define COL9 28.575
#define COL10 31.75
#define COL11 34.925
#define COL12 38.1
#define COL13 41.275
#define COL14 44.45
#define COL15 47.625
#define COL16 50.8
#define COL17 53.975
#define COL18 57.15
#define COL19 60.325
#define COL20 63.5
#define COL21 66.675
#define COL22 69.85
#define COL23 73.025
#define COL24 76.2
#define COL25 79.375
#define COL26 82.55
#define COL27 85.725
#define COL28 88.9
#define COL29 92.075
#define COL30 95.25
#define COL31 98.425


#define ROW0 0
#define ROW1 4.016
#define ROW2 8.032
#define ROW3 12.048
#define ROW4 16.064
#define ROW5 20.08
#define ROW6 24.096
#define ROW7 28.112
#define ROW8 32.128
#define ROW9 36.144
#define ROW10 40.16
#define ROW11 44.176
#define ROW12 48.192
#define ROW13 52.208
#define ROW14 56.224
#define ROW15 60.24
#define ROW16 64.256
#define ROW17 68.272
#define ROW18 72.288
#define ROW19 76.304
#define ROW20 80.32
#define ROW21 84.336
#define ROW22 88.352
#define ROW23 92.368
#define ROW24 96.384
#define ROW25 100.4
#define ROW26 104.416
#define ROW27 108.432
#define ROW28 112.448
#define ROW29 116.464
#define ROW30 120.48
#define ROW31 124.496


/*
PHP rogram for generating define statements
<?PHP

$number_of_columns = 32;
$number_of_rows = 32;
$first_column_guide_position_x = 3.175;
$first_row_guide_position_y = 4.016;

for($i = 0; $i < $number_of_columns; $i++)
{
  print("#define COL${i} " . ($i * $first_column_guide_position_x) . "\n");
}

for($i = 0; $i < $number_of_rows; $i++)
{
  print("#define ROW${i} " . ($i * $first_row_guide_position_y) . "\n");
}

?>
*/
