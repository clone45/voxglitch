#define GAIN 5.0
#define NUMBER_OF_SAMPLES 8
#define NUMBER_OF_STEPS 16

const int NUMBER_OF_MEMORY_SLOTS = 16;
const int MAX_SEQUENCER_STEPS = 16;

// Constants for patterns
const float DRAW_AREA_WIDTH = 400.0;
const float DRAW_AREA_HEIGHT = 143.11;
const float BAR_HEIGHT = 143.11;
const float BAR_HORIZONTAL_PADDING = .8;
const float DRAW_AREA_POSITION_X = 157.778;
const float DRAW_AREA_POSITION_Y = 61.0;

const float TABS_POSITION_Y = 48.8;

const float OVERLAY_WIDTH = 400.0;

const int NUMBER_OF_TABS = 6;
const int NUMBER_OF_SEQUENCERS = 6;
const float LCD_TABS_HEIGHT = 20.0;
const float LCD_TABS_WIDTH = 65.7;
const float TABS_HORIZONTAL_PADDING = 1.0;

const float ratchet_divisions[5] = {
    4.0, // 4 == double ratchet
    6.0, // 6 == triplet ratchet
    8.0, // 8 == quad ratchet
    12.0, // ??
    16.0 // etc
};
      
      
      