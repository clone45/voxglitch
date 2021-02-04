struct HazumiSequencer
{
  unsigned int trigger_options[SEQUENCER_COLUMNS] = {0,0,0,0,0,0,0,0};

  // Start the balls at the bottom
  unsigned int ball_locations[8] = { 0,0,0,0,0,0,0,0 };

  bool ball_directions[8] = { 1,1,1,1,1,1,1,1 }; // 0 == down, 1 == up
  unsigned int column_heights[SEQUENCER_COLUMNS] = { 1,1,1,1,1,1,1,1 };

  // Store the trigger results in the sequencer.  This will be used by
  // the widget to color the balls differently when they trigger an output
  bool stored_trigger_results[SEQUENCER_COLUMNS] = { 0,0,0,0,0,0,0,0 };

  // constructor
  HazumiSequencer()
  {
  }

  // Step the sequencer and return, as separate booleans, if any of the five
  // trigger groups have been triggered.

  void step(bool *trigger_results)
  {
    bool hit_top = false;
    bool hit_bottom = false;

    for(unsigned int i=0; i<8; i++)
    {
      hit_top = false;
      hit_bottom = false;
      trigger_results[i] = false;

      if(column_heights[i] == 1)
      {
        ball_directions[i] = 1;
        ball_locations[i] = 0;
      }
      else if(ball_locations[i] == 0)
      {
          ball_directions[i] = 1;
          ball_locations[i] += 1;
      }
      else if(ball_locations[i] > (column_heights[i])) // column height has been resized and ball is out of bounds
      {
        ball_locations[i] = column_heights[i];
        ball_directions[i] = 0;
      }
      else if(ball_locations[i] == (column_heights[i] - 1))
      {
        ball_directions[i] = 0;
        ball_locations[i] -= 1;
      }
      else if(ball_directions[i] == 0)
      {
        ball_locations[i] -= 1;
      }
      else
      {
        ball_locations[i] += 1;
      }

      //
      // After the ball has been moved, see if it should trigger an output
      //

      if(ball_locations[i] == 0 && column_heights[i] != 1) hit_bottom = true;
      if((ball_locations[i] > 0) && (ball_locations[i] == (column_heights[i] - 1))) hit_top = true;

      if(trigger_options[i] == TRIGGER_AT_BOTTOM && hit_bottom) trigger_results[i] = true;
      if(trigger_options[i] == TRIGGER_AT_TOP && hit_top) trigger_results[i] = true;
      if(trigger_options[i] == TRIGGER_AT_BOTH && (hit_bottom || hit_top)) trigger_results[i] = true;

      stored_trigger_results[i] = trigger_results[i];
    }
  }


  void reset()
  {
    for(unsigned int i=0; i<8; i++)
    {
      ball_locations[i] = 0;
    }
  }

};
