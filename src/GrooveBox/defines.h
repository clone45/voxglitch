namespace groove_box
{
  const int NUMBER_OF_STEPS = 16;
  const int NUMBER_OF_TRACKS = 8;
  const int NUMBER_OF_MEMORY_SLOTS = 16;
  const int NUMBER_OF_FUNCTIONS = 8;
  const int NUMBER_OF_OFFSET_SNAP_OPTIONS = 8;
  const int NUMBER_OF_RATCHET_PATTERNS = 16;

  const int FUNCTION_VOLUME = 0;
  const int FUNCTION_PAN = 1;
  const int FUNCTION_PITCH = 2;
  const int FUNCTION_RATCHET = 3;
  const int FUNCTION_OFFSET = 4;
  const int FUNCTION_PROBABILITY = 5;
  const int FUNCTION_LOOP = 6;
  const int FUNCTION_REVERSE = 7;

  const float default_volume = 0.5;
  const float default_pan = 0.5;
  const float default_pitch = 0.5;
  const float default_ratchet = 0.0;
  const float default_offset = 0.0;
  const float default_probability = 1.0;
  const float default_loop = 0.0;
  const bool default_reverse = false;

  const std::string PLACEHOLDER_TRACK_NAMES[NUMBER_OF_TRACKS] = {
    "kick_drum.wav",
    "snare.wav",
    "hihat_open.wav",
    "hihat_closed.wav",
    "low_tom.wav",
    "jiggly_puff.wav",
    "",
    ""
  };

  const std::string offset_snap_names[NUMBER_OF_OFFSET_SNAP_OPTIONS] = {
    "none",
    "4",
    "8",
    "16",
    "32",
    "64",
    "128",
    "256"
  };

  const unsigned int offset_snap_values[NUMBER_OF_OFFSET_SNAP_OPTIONS] = {
    0,
    4,
    8,
    16,
    32,
    64,
    128,
    256
  };

}
