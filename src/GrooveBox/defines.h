namespace groove_box
{
  const int NUMBER_OF_STEPS = 16;
  const int NUMBER_OF_TRACKS = 8;
  const int NUMBER_OF_MEMORY_SLOTS = 16;
  const int NUMBER_OF_FUNCTIONS = 8;

  const int FUNCTION_VOLUME = 0;
  const int FUNCTION_PAN = 1;
  const int FUNCTION_PITCH = 2;
  const int FUNCTION_RATCHET = 3;
  const int FUNCTION_OFFSET = 4;
  const int FUNCTION_PROBABILITY = 5;
  const int FUNCTION_LOOP = 6;
  const int FUNCTION_REVERSE = 7;

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

}
