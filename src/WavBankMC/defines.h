#define GAIN 5.0

// There's not many times that you would want smoothing disabled, so to make
// room on the front panel for the loop switch, I'm simply going to set
// smoothing to true.  If there's demand, I'll add an option to the right-click
// context menu for enabling and disabling smoothing.

// #define SMOOTH_ENABLED 1

#define NUMBER_OF_CHANNELS 16

#define NUMBER_OF_SAMPLE_DISPLAY_ROWS 21

#define READOUT_WIDTH 110
#define READOUT_HEIGHT 330


// Constants for trig_input_response_mode
#define TRIGGER 0
#define GATE 1

#define RESTART_PLAYBACK 0
#define CONTINUAL_PLAYBACK 1
