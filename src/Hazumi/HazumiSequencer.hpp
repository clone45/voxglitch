struct HazumiSequencer
{
  // Start the balls at the bottom
  unsigned int ball_locations[8] = {
    1,2,5,6,0,0,0,0
  };

  // 0 == down
  // 1 == up
  bool ball_directions[8] = {
    1,1,1,1,1,1,1,1
  };

  unsigned int column_heights[8] = {
    4,8,12,16,10,2,15,15
  };

  // constructor
  HazumiSequencer()
  {
  }

  // Step the sequencer and return, as separate booleans, if any of the five
  // trigger groups have been triggered.

  void step(bool *trigger_results)
  {
    for(unsigned int i=0; i<8; i++)
    {
      // if (ball_locations[i] > 15) ball_locations[i] = 15;
      // if (ball_locations[i] < 1) ball_locations[i] = 1;

      // unsigned int h = column_heights[i];



      if(column_heights[i] == 1)
      {
        // DEBUG(std::to_string(h).c_str());
        DEBUG("got here");
        // don't do anything
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

      // if (ball_locations[i] > 15) ball_locations[i] = 15;
      // if (ball_locations[i] < 1) ball_locations[i] = 1;

      // After the ball has been moved, see if it's at the bottom of the
      // sequencer.  If so, set the corresponding trigger result.  If not,
      // clear the corresponding trigger result.
      if(ball_locations[i] == 0 && column_heights[i] != 1)
      {
        trigger_results[i] = true;
      }
      else
      {
        trigger_results[i] = false;
      }
    }

  }
};
