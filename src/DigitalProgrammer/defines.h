namespace digital_programmer
{

// Constants
const float DRAW_AREA_WIDTH = 24.0;
const unsigned int NUMBER_OF_SLIDERS = 16;
const float SLIDER_HEIGHT = 310.0;
const float SLIDER_WIDTH = 24.0;
const float SLIDER_HORIZONTAL_PADDING = .8;
const float DRAW_AREA_POSITION_X = 9;
const float DRAW_AREA_POSITION_Y = 30;
const unsigned int NUMBER_OF_BANKS = 24;
const float BANK_BUTTON_WIDTH = 30.118;
const float BANK_BUTTON_HEIGHT = 30.118;
const float BANK_BUTTONS_X = 189.76;
const float BANK_BUTTONS_Y = 47.141;
const float BANK_BUTTONS_PADDING = 1.8;
const unsigned int NUMBER_OF_SNAP_DIVISIONS = 5;
const unsigned int NUMBER_OF_VOLTAGE_RANGES = 8;

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
