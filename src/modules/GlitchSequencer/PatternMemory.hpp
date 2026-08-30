// One memory slot: a complete pattern — the seed, all 8 trigger layers, and
// the sequence length.  Switching memories moves the LEN knob to the slot's
// stored length (params[].setValue, same as the GrooveBox's memory buttons).

struct PatternMemory
{
  bool seed[SEQUENCER_ROWS][SEQUENCER_COLUMNS] = {};
  bool triggers[NUMBER_OF_TRIGGER_GROUPS][SEQUENCER_ROWS][SEQUENCER_COLUMNS] = {};
  unsigned int length = 16;

  void clear()
  {
    length = 16;
    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        seed[row][column] = 0;
        for(unsigned int i = 0; i < NUMBER_OF_TRIGGER_GROUPS; i++) triggers[i][row][column] = 0;
      }
    }
  }

  void copyFrom(PatternMemory *src)
  {
    length = src->length;
    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        seed[row][column] = src->seed[row][column];
        for(unsigned int i = 0; i < NUMBER_OF_TRIGGER_GROUPS; i++) triggers[i][row][column] = src->triggers[i][row][column];
      }
    }
  }
};
