namespace scalar_110
{
  const int NUMBER_OF_STEPS = 16;
  const int NUMBER_OF_TRACKS = 8;
  const int NUMBER_OF_MEMORY_SLOTS = 16;

  const int NUMBER_OF_PARAMETERS = 8;
  const int NUMBER_OF_SAMPLES = 16;

  const int NUMBER_OF_FUNCTIONS = 8;

  const int FUNCTION_VOLUME = 0;
  const int FUNCTION_PAN = 1;
  const int FUNCTION_PITCH = 2;
  const int FUNCTION_RATCHET = 3;
  const int FUNCTION_OFFSET = 4;
  const int FUNCTION_PROBABILITY = 5;
  const int FUNCTION_LOOP = 6;
  const int FUNCTION_REVERSE = 7;

  const float LABEL_POSITIONS[NUMBER_OF_TRACKS][2] = {
    {380.007812 + 20, 83 - 11},
    {380.007812 + 20, 114.5 - 11},
    {379.007812 + 20, 145.011719 - 11},
    {380.007812 + 20, 177.011719 - 11},
    {507.007812 + 20, 83 - 11},
    {507.007812 + 20, 114.5 - 11},
    {507.007812 + 20, 145.011719 - 11},
    {507.007812 + 20, 177.011719 - 11}
  };

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
