struct CellularAutomatonSequencer
{
  unsigned int position = 0;
  unsigned int length = 0;

  bool seed[SEQUENCER_ROWS][SEQUENCER_COLUMNS] = {
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,1,1, 1,1,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,1, 1,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
  };

  bool state[SEQUENCER_ROWS][SEQUENCER_COLUMNS];
  bool next[SEQUENCER_ROWS][SEQUENCER_COLUMNS];
  bool triggers[NUMBER_OF_TRIGGER_GROUPS][SEQUENCER_ROWS][SEQUENCER_COLUMNS] =
  {
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    },
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,1,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    },
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    },
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,1,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    },
    {
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 1,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,1,0, 0,1,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,1,1, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0 },
      { 0,0,0,1, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
      { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
    }
  };

  // constructor
  CellularAutomatonSequencer()
  {
    clearPattern(&state);
    clearPattern(&next);

    copyPattern(&state, &seed);
  }

  // Step the sequencer and return, as separate booleans, if any of the five
  // trigger groups have been triggered.

  void step(bool *trigger_results)
  {
    position ++;

    if(position >= length)
    {
      restart_sequence();
    }
    else
    {
      calculate_next_state(trigger_results);
    }
  }

  void reset()
  {
    restart_sequence();
  }

  void restart_sequence()
  {
    position = 0;
    copyPattern(&state, &seed);  // dst < src
    clearPattern(&next);
  }

  void calculate_next_state(bool *trigger_results)
  {
    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      trigger_results[i] = false;
    }

    for(unsigned int row = 1; row < SEQUENCER_ROWS - 1; row++)
    {
      for(unsigned int column = 1; column < SEQUENCER_COLUMNS - 1; column++)
      {
        unsigned int neighbors = 0;

        // Calculate number of neighbors
        if(state[row-1][column-1]) neighbors++;
        if(state[row-1][column]) neighbors++;
        if(state[row-1][column+1]) neighbors++;
        if(state[row][column-1]) neighbors++;
        if(state[row][column+1]) neighbors++;
        if(state[row+1][column-1]) neighbors++;
        if(state[row+1][column]) neighbors++;
        if(state[row+1][column+1]) neighbors++;

        // Apply game of life rules
        if(state[row][column] && neighbors < 2) next[row][column] = 0;
        else if(state[row][column] && neighbors > 3) next[row][column] = 0;
        else if(state[row][column] && (neighbors == 2 || neighbors == 3)) next[row][column] = 1;
        else if(state[row][column] == 0 && neighbors == 3) next[row][column] = 1;

        // Detect when to output a trigger!
        if(state[row][column] == 0 && next[row][column] == 1)
        {
          for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
          {
            if(triggers[i][row][column]) trigger_results[i] = true;
          }
        }
      }
    }

    copyPattern(&state, &next); // dst < src
  }

  void copyPattern(bool (*dst)[SEQUENCER_ROWS][SEQUENCER_COLUMNS], bool (*src)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
  {
    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        (*dst)[row][column] = (*src)[row][column];
      }
    }
  }

  void clearPattern(bool (*pattern)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
  {
    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        (*pattern)[row][column] = 0;
      }
    }
  }

  void setLength(unsigned int length)
  {
    this->length = length;
  }

  //
  // unsigned int packPattern(pointer to a pattern)
  //
  // Used to compress the boolean array into an integer for saving to a patch

  std::string packPattern(bool (*pattern)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
  {
    std::string packed_pattern_data = "";

    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        if((*pattern)[row][column] == 1)
        {
          packed_pattern_data = packed_pattern_data + '1';
        }
        else
        {
          packed_pattern_data = packed_pattern_data + '0';
        }
      }
    }

    return(packed_pattern_data);
  }

  //
  // void unpackPattern(pointer to a pattern)
  //
  // Used to uncompress the integer created from packPattern when loading
  // a patch.

  void unpackPattern(std::string packed_pattern_data, bool (*pattern)[SEQUENCER_ROWS][SEQUENCER_COLUMNS])
  {
    unsigned int string_index = 0;

    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        if(packed_pattern_data[string_index] == '0')
        {
          (*pattern)[row][column] = 0;
        }
        else
        {
          (*pattern)[row][column] = 1;
        }

        string_index++;
      }
    }
  }
};
