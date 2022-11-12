namespace groove_box
{
  const int NUMBER_OF_STEPS = 16;
  const int NUMBER_OF_TRACKS = 8;
  const int NUMBER_OF_MEMORY_SLOTS = 16;
  const int NUMBER_OF_FUNCTIONS = 16;
  const int NUMBER_OF_SAMPLE_POSITION_SNAP_OPTIONS = 8;
  const int NUMBER_OF_RATCHET_PATTERNS = 16;

  const int FUNCTION_VOLUME = 0;
  const int FUNCTION_PAN = 1;
  const int FUNCTION_PITCH = 2;
  const int FUNCTION_RATCHET = 3;
  const int FUNCTION_PROBABILITY = 4;
  const int FUNCTION_DELAY_MIX = 5;
  const int FUNCTION_DELAY_LENGTH = 6;
  const int FUNCTION_DELAY_FEEDBACK = 7;
  const int FUNCTION_ATTACK = 8;
  const int FUNCTION_RELEASE = 9;
  const int FUNCTION_LOOP = 10;
  const int FUNCTION_REVERSE = 11;
  const int FUNCTION_SAMPLE_START = 12;
  const int FUNCTION_SAMPLE_END = 13;
  const int FUNCTION_FILTER_CUTOFF = 14;
  const int FUNCTION_FILTER_RESONANCE = 15;

  const float MODULE_WIDTH = 223.52000 * 2.952756;
  const float MODULE_HEIGHT = 128.50000 * 2.952756;

  const float maximum_release_time = 4.0;

  
  enum Parameters
  {
    VOLUME,
    PAN,
    PITCH,
    RATCHET,
    PROBABILITY, 
    DELAY_MIX,
    DELAY_LENGTH,
    DELAY_FEEDBACK,       
    ATTACK,
    RELEASE,
    LOOP,
    REVERSE,
    SAMPLE_START,
    SAMPLE_END,
    FILTER_CUTOFF,
    FILTER_RESONANCE
  };


  const float default_parameter_values[NUMBER_OF_FUNCTIONS] = {
      0.5, // 0) default volume
      0.5, // 1) default pan
      0.5, // 2) default pitch
      0.0, // 3) default ratchet
      1.0, // 4) default probability
      0.0, // 5) default delay mix
      0.5, // 6) default delay length
      0.5, // 7) default delay feedback
      0.0, // 8) default attack
      1.0, // 9) default release
      0.0, // 10) default loop
      0.0, // 11) default reverse      
      0.0, // 12) default sample_start
      1.0, // 13) default sample_end
      1.0, // 14) default filter cutoff
      0.0  // 15) default filter resonance
  };

  const std::string FUNCTION_NAMES[NUMBER_OF_FUNCTIONS] = {
      "Volume",
      "Pan",
      "Pitch",
      "Ratchet",
      "Probability",
      "Delay Mix",
      "Delay Length",
      "Delay Feedback",
      "Attack",
      "Release",
      "Loop",
      "Reverse",
      "Sample Start",
      "Sample End",
      "Filter Cutoff",
      "Filter Resonance"};

  const std::string PLACEHOLDER_TRACK_NAMES[NUMBER_OF_TRACKS] = {
      "kick_drum.wav",
      "snare.wav",
      "hihat_open.wav",
      "hihat_closed.wav",
      "low_tom.wav",
      "jiggly_puff.wav",
      "",
      ""};

  const std::string sample_position_snap_names[NUMBER_OF_SAMPLE_POSITION_SNAP_OPTIONS] = {
      "none",
      "4",
      "8",
      "16",
      "32",
      "64",
      "128",
      "256"};

  const unsigned int sample_position_snap_values[NUMBER_OF_SAMPLE_POSITION_SNAP_OPTIONS] = {
      0,
      4,
      8,
      16,
      32,
      64,
      128,
      256};

  const bool ratchet_patterns[NUMBER_OF_RATCHET_PATTERNS][7] = {
      {0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 1, 0, 0, 0},
      {0, 0, 0, 1, 1, 1, 1},
      {0, 0, 1, 0, 0, 1, 0},
      {0, 1, 1, 0, 1, 1, 0},
      {0, 1, 0, 1, 0, 1, 0},
      {0, 1, 0, 0, 1, 0, 0},
      {1, 0, 1, 0, 1, 0, 1},
      {1, 1, 0, 0, 0, 1, 1},
      {1, 1, 0, 1, 1, 0, 1},
      {1, 1, 1, 0, 1, 0, 0},
      {1, 1, 1, 0, 1, 0, 1},
      {1, 1, 1, 0, 0, 0, 0},
      {1, 1, 1, 1, 0, 0, 0},
      {1, 1, 1, 0, 1, 1, 1},
      {1, 1, 1, 1, 1, 1, 1}};

  const float button_positions_y = mm2px(88.5);

  const float button_positions[16][2] = {
      {mm2px(9.941), button_positions_y},
      {mm2px(23.52), button_positions_y},
      {mm2px(37.10), button_positions_y},
      {mm2px(50.69), button_positions_y},
      {mm2px(64.27), button_positions_y},
      {mm2px(77.85), button_positions_y},
      {mm2px(91.43), button_positions_y},
      {mm2px(105.02), button_positions_y},
      {mm2px(118.60), button_positions_y},
      {mm2px(132.18), button_positions_y},
      {mm2px(145.76), button_positions_y},
      {mm2px(159.35), button_positions_y},
      {mm2px(172.93), button_positions_y},
      {mm2px(186.51), button_positions_y},
      {mm2px(200.09), button_positions_y},
      {mm2px(213.67), button_positions_y}};

}

    /* previous version:
  enum Parameters
  {
    VOLUME,
    PAN,
    PITCH,
    RATCHET,
    SAMPLE_START,
    SAMPLE_END,
    PROBABILITY,
    LOOP,
    REVERSE,
    ATTACK,
    RELEASE,
    DELAY_MIX,
    DELAY_LENGTH,
    DELAY_FEEDBACK,
    FILTER_CUTOFF,
    FILTER_RESONANCE
  };

  const float default_volume = 0.5;
  const float default_pan = 0.5;
  const float default_pitch = 0.5;
  const float default_ratchet = 0.0;
  const float default_sample_start = 0.0;
  const float default_sample_end = 1.0;
  const float default_probability = 1.0;
  const float default_loop = 0.0;
  const bool default_reverse = false;
  const float default_attack = 0.0;
  const float default_release = 1.0;
  const float default_delay_mix = 0.0;
  const float default_delay_length = 0.5;
  const float default_delay_feedback = 0.5;
  const float default_filter_cutoff = 1.0;
  const float default_filter_resonance = 0.0;
  */