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

    if((playing == false) || (sample_position >= this->sample.total_sample_count) || (sample.loaded == false)) return { 0,0 };

		return { this->sample.leftPlayBuffer[sample_position], this->sample.rightPlayBuffer[sample_position] };
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

      // DEBUG(std::to_string(playback_position).c_str());

      // If the playback position is past the playback length, end sample playback
  		if(playback_position >= sample.total_sample_count) stop();
    }
	}

  void loadSample(std::string path)
  {
    sample.load(path, false);
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
