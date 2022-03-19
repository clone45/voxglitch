namespace scalar_110
{

struct Track
{
  bool steps[NUMBER_OF_STEPS] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
  unsigned int playback_position = 0;
  Engine *engine = new Foo();
  unsigned int engine_index = 0; // must exist to save/load engine selection
  unsigned int old_engine_index = 0;
  StepParams step_params[NUMBER_OF_STEPS];
  StepParams default_step_params;
  unsigned int track_number = 0;

  Track()
  {
    copyEngineDefaults();
  }

  void step()
  {
    playback_position = (playback_position + 1) % NUMBER_OF_STEPS;

    if(playback_position == 0) engine->reset();

    if (steps[playback_position])
    {
      engine->trigger(& step_params[playback_position]);
    }
  }

  unsigned int getPosition()
  {
    return(playback_position);
  }

  StepParams *getParameters(unsigned int selected_step)
  {
    return(& this->step_params[selected_step]);
  }

  float getParameter(unsigned int selected_step, unsigned int parameter_number)
  {
    return(this->step_params[selected_step].p[parameter_number]);
  }

  void setParameter(unsigned int selected_step, unsigned int parameter_number, float value)
  {
    this->step_params[selected_step].p[parameter_number] = value;
  }

  float getDefaultParameter(unsigned int parameter_number)
  {
    return(default_step_params.p[parameter_number]);
  }

  /*
  void setDefaultParameter(unsigned int parameter_number, float value)
  {
    this->default_step_params.p[parameter_number] = value;
  }
  */

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

  void setEngine(unsigned int engine_index)
  {
    this->engine_index = engine_index;

    // If the engine selection has changed, then assign the newly selected
    // engine to the currently selected track.
    if(engine_index != old_engine_index)
    {
      Engine *engine_to_delete = NULL;
      if(engine != NULL) engine_to_delete = engine;

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

      if(engine_to_delete != NULL) delete engine_to_delete;
    }

    // Consider copying engine defaults here instead of wherever it's done now
  }

  // Copy the engine default step parameters into the track's memory.
  // When the user swiches tracks, the default step parameters will be
  // restored to where the user left them.
  void copyEngineDefaults()
  {
    StepParams *engine_default_parameters = engine->getDefaultParams();

    for(unsigned int step_number=0; step_number<NUMBER_OF_STEPS; step_number++)
    {
      for(unsigned int parameter_number=0; parameter_number<NUMBER_OF_PARAMETERS; parameter_number++)
      {
        // setParameter(unsigned int selected_step, unsigned int parameter_number, float value)
        this->setParameter(step_number, parameter_number, engine_default_parameters->p[parameter_number]);
      }
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
