
const int NUMBER_OF_SEQUENCERS = 16;
const int MAX_SEQUENCER_STEPS = 32;
const int NUMBER_OF_VOLTAGE_RANGES = 8;
const int NUMBER_OF_SNAP_DIVISIONS = 8;

// Constants for patterns
const int DRAW_AREA_WIDTH = 486;
const int DRAW_AREA_HEIGHT = 214;
const float BAR_HEIGHT = 214.0;
const float BAR_HORIZONTAL_PADDING = .8;
const float DRAW_AREA_POSITION_X = 9.9;
const float DRAW_AREA_POSITION_Y = 9.5 + .9;

// Constants for gate sequencer
const float GATES_DRAW_AREA_WIDTH = 486.0;
const float GATES_DRAW_AREA_HEIGHT = 16.0;
const float GATES_DRAW_AREA_POSITION_X = 9.9;
const float GATES_DRAW_AREA_POSITION_Y = 86 + .9;
const float GATE_BAR_HEIGHT = 16.0;

const float TOOLTIP_WIDTH = 33.0;
const float TOOLTIP_HEIGHT = 20.0;

const float OVERLAY_WIDTH = 479;


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

double snap_divisions[NUMBER_OF_SNAP_DIVISIONS] = { 0,8,10,12,16,24,32,26 };
