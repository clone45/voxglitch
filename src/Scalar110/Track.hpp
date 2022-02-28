namespace scalar_110
{

struct Track
{
  bool steps[NUMBER_OF_STEPS] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
  unsigned int playback_position = 0;
  Engine *engine = new Foo();
  unsigned int engine_index = 0; // must exist to save/load engine selection
  unsigned int old_engine_index = 0;
  StepParams step_parameters[NUMBER_OF_STEPS];
  unsigned int track_number = 0;

  void step()
  {
    playback_position = (playback_position + 1) % NUMBER_OF_STEPS;

    if (steps[playback_position])
    {
      engine->trigger(& step_parameters[playback_position]);
    }
  }

  unsigned int getPosition()
  {
    return(playback_position);
  }

  StepParams *getParameters(unsigned int selected_step)
  {
    return(& this->step_parameters[selected_step]);
  }

  float getParameter(unsigned int selected_step, unsigned int parameter_number)
  {
    return(this->step_parameters[selected_step].p[parameter_number]);
  }

  void setParameter(unsigned int selected_step, unsigned int parameter_number, float value)
  {
    this->step_parameters[selected_step].p[parameter_number] = value;
  }

  void setPosition(unsigned int playback_position)
  {
    if(playback_position < NUMBER_OF_STEPS)
    {
      this->playback_position = playback_position;
    }
  }

  void toggleStep(unsigned int i)
  {
    steps[i] ^= true;
  }

  bool getValue(unsigned int i)
  {
    return(steps[i]);
  }

  void setValue(unsigned int i, bool value)
  {
    steps[i] = value;
  }

  void reset()
  {
    playback_position = 0;
  }

  // To do: Migrate all engine stuff over here
  void setEngine(unsigned int engine_index)
  {
    this->engine_index = engine_index;

    // If the engine selection has changed, then assign the newly selected
    // engine to the currently selected track.
    if(engine_index != old_engine_index)
    {
      if(engine != NULL) delete engine;

      switch(engine_index) {
        case 0:
          engine = new Foo(); // Equation player
          break;
        case 1:
          engine = new LowDrums(); // 8-bit drums
          break;
        case 2:
          engine = new Sampler(track_number); // Sample player
          break;
      }
      old_engine_index = engine_index;
    }
  }

  unsigned int getEngine()
  {
    return this->engine_index;
  }

  std::pair<float, float> process()
  {
    float left_output;
    float right_output;

    std::tie(left_output, right_output) = this->engine->process();
    return { left_output, right_output };
  }

};

}
