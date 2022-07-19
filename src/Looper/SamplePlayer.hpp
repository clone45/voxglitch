struct SamplePlayer
{
	// sample_ptr points to the loaded sample in memory
	Sample sample;
  bool playing = true;
	double playback_position = 0;
  double step_amount = APP->engine->getSampleRate();
  unsigned int sample_position = 0;

  void getStereoOutput(float *left_output, float *right_output, unsigned int interpolation)
	{
    unsigned int sample_index = playback_position; // convert float to int

    if((playing == false) || ((unsigned int) sample_index >= this->sample.size()) || (sample.loaded == false))
    {
      *left_output = 0;
      *right_output = 0;
    }
    else
    {
      if(interpolation == 0)
      {
        // Normal version, using sample index
        this->sample.read(sample_index, left_output, right_output);
      }
      else
      {
        // Read sample using Linear Interpolation, sending in double
        this->sample.readLI(playback_position, left_output, right_output);
      }
    }
	}

  void updateSampleRate(float rack_sample_rate)
  {
    this->step_amount = sample.sample_rate / rack_sample_rate;
  }

	void step()
	{
    if(playing && sample.loaded)
    {
      // Step the playback position forward.
  		playback_position = playback_position + this->step_amount;

      // If the playback position is past the playback length, loop playback
  		if(playback_position >= sample.size()) playback_position = 0;
    }
	}

  void trigger()
  {
    playback_position = 0;
    playing = true;
  }

  void stop()
  {
    playing = false;
  }

  void loadSample(std::string path)
  {
    sample.load(path);
    this->trigger();
  }

  void reset()
  {
    playback_position = 0;
  }

  void setPositionFromInput(float position_input_value) // 1 to 10
  {
    position_input_value = clamp(position_input_value, 0.0, 10.0);
    playback_position = rescale(position_input_value, 0.0, 10.0, 0.0, sample.size());
  }

  std::string getFilename()
  {
    return(sample.filename);
  }

  std::string getPath()
  {
    return(sample.path);
  }
};
