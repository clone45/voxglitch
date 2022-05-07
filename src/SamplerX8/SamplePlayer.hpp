struct SamplePlayer
{
	// sample_ptr points to the loaded sample in memory
	Sample sample;
	float playback_position = 0.0f;
  unsigned int sample_position = 0;
  bool playing = false;

	std::pair<float, float> getStereoOutput()
	{
    sample_position = playback_position; // convert float to int

    float left; float right;

    if((playing == false) || (sample_position >= this->sample.size()) || (sample.loaded == false))
    {
      left = 0;
      right = 0;
    }

    this->sample.read(sample_position, &left, &right);
		return { left, right };
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

	void step(float rack_sample_rate)
	{
    if(playing && sample.loaded)
    {
      float step_amount = sample.sample_rate / rack_sample_rate;

      // Step the playback position forward.
  		playback_position = playback_position + step_amount;

      // If the playback position is past the playback length, end sample playback
  		if(playback_position >= sample.size()) stop();
    }
	}

  void loadSample(std::string path)
  {
    sample.load(path);
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
