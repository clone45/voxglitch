#define GAIN 5.0

// There's not many times that you would want smoothing disabled, so to make
// room on the front panel for the loop switch, I'm simply going to set
// smoothing to true.  If there's demand, I'll add an option to the right-click
// context menu for enabling and disabling smoothing.

// #define SMOOTH_ENABLED 1

#define NUMBER_OF_CHANNELS 16

#define NUMBER_OF_SAMPLE_DISPLAY_ROWS 23

#define READOUT_WIDTH 110
#define READOUT_HEIGHT 360


// Constants for trig_input_response_mode
#define TRIGGER 0
#define GATE 1

#define RESTART_PLAYBACK 0
#define CONTINUAL_PLAYBACK 1

// Column and row grid system
#define COL0 0
#define COL1 5.715
#define COL2 11.43
#define COL3 17.145
#define COL4 22.86
#define COL5 28.575
#define COL6 34.29
#define COL7 40.005
#define COL8 45.72
#define COL9 51.435
#define COL10 57.15
#define COL11 62.865
#define COL12 68.58
#define COL13 74.295
#define COL14 80.01
#define COL15 85.725
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

  for($i = 0; $i < 16; $i++)
  {
    print("#define COL${i} " . ($i * 5.715) . "\n");
  }

  for($i = 0; $i < 32; $i++)
  {
    print("#define ROW${i} " . ($i * 4.016) . "\n");
  }

?>
*/
