#define MAX_GRAVEYARD_CAPACITY 120.0f
#define MAX_GHOST_SPAWN_RATE 30000.0f

const float WAVEFORM_WIDGET_HEIGHT = 100.0;
const float WAVEFORM_WIDGET_WIDTH = 190.0;

float modes[4][6] = {
  // Default mode
  {
    2.0,    // maximum playback length (1/2 of a second)
    256.0,  // minimum playback length (1/256 of a second)
    500.0,  // maximum rate (ghosts per second)
    10.0,   // minimum rate (ghosts per second)
    1024.0, // jitter spread
    0.0     // removal method: 0==normal, 1==random
  },
  // Second mode
  {
    2.0,    // maximum playback rate
    512.0,  // minimum playback rate
    200.0,  // maximum gps (ghosts per second)
    5.0,    // minimum ghosts per second
    256.0,  // jitter spread
    0.0 // removal method: 0==normal, 1==random
  },
  // Third mode
  {
    2.0,    // maximum playback length (1/2 of a second)
    512.0,  // minimum playback length (1/256 of a second)
    300.0,  // maximum rate (ghosts per second)
    40.0,   // minimum rate (ghosts per second)
    8192.0, // jitter spread (high)
    0.0 // removal method: 0==normal, 1==random
  },
  // Fourth mode
  {
    2.0,    // maximum playback length (1/2 of a second)
    512.0,  // minimum playback length (1/256 of a second)
    300.0,  // maximum rate (ghosts per second)
    40.0,   // minimum rate (ghosts per second)
    1024.0, // jitter spread
    1.0 // removal method: 0==normal, 1==random
  },
};
