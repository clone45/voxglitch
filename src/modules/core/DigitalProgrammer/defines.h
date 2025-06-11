namespace digital_programmer
{

// Constants
const float DRAW_AREA_WIDTH = 24.0;
const unsigned int NUMBER_OF_SLIDERS = 16;
const float SLIDER_HEIGHT = 288.5;
const float SLIDER_WIDTH = 24.0;
const float SLIDER_HORIZONTAL_PADDING = .8;
const float DRAW_AREA_POSITION_X = 9;
const float DRAW_AREA_POSITION_Y = 30;
const unsigned int NUMBER_OF_BANKS = 24;
const float BANK_BUTTON_WIDTH = 25.20;
const float BANK_BUTTON_HEIGHT = 25.20;
const float BANK_BUTTONS_PADDING = 1.8;
const unsigned int NUMBER_OF_SNAP_DIVISIONS = 5;
const unsigned int NUMBER_OF_VOLTAGE_RANGES = 8;

const unsigned int SELECT_MODE = 0;
const unsigned int COPY_MODE = 1;
const unsigned int CLEAR_MODE = 2;
const unsigned int RANDOMIZE_MODE = 3;


double voltage_ranges[NUMBER_OF_VOLTAGE_RANGES][2] = {
  { 0.0, 10.0 },
  { -10.0, 10.0 },
  { 0.0, 5.0 },
  { -5.0, 5.0 },
  { 0.0, 3.0 },
  { -3.0, 3.0 },
  { 0.0, 1.0},
  { -1.0, 1.0}
};

}
