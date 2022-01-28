#define NUMBER_OF_SEQUENCERS 6
#define MAX_SEQUENCER_STEPS 32
#define NUMBER_OF_VOLTAGE_RANGES 8
#define NUMBER_OF_SNAP_DIVISIONS 8

// Constants for patterns
#define DRAW_AREA_WIDTH 486.0
#define DRAW_AREA_HEIGHT 214.0
#define BAR_HEIGHT 214.0
#define BAR_HORIZONTAL_PADDING .8
#define DRAW_AREA_POSITION_X 9
#define DRAW_AREA_POSITION_Y 9.5

// Constants for gate sequencer
#define GATES_DRAW_AREA_WIDTH 486.0
#define GATES_DRAW_AREA_HEIGHT 16.0
#define GATES_DRAW_AREA_POSITION_X 9
#define GATES_DRAW_AREA_POSITION_Y 86
#define GATE_BAR_HEIGHT 16.0

#define TOOLTIP_WIDTH 33.0
#define TOOLTIP_HEIGHT 20.0

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
