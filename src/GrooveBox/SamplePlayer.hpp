namespace groove_box
{

struct SamplePlayer
{
	Sample sample;
	float playback_position = 0.0f;
  unsigned int sample_position = 0;
  bool playing = false;

	std::pair<float, float> getStereoOutput()
	{
    sample_position = playback_position; // convert float to int

    // If the sample is not playing, return 0's
    if((playing == false) || (sample_position >= this->sample.size()) || (sample.loaded == false)) return { 0,0 };

    // Read the sample at the sample position and return the value
    return(this->sample.read(sample_position));
	}

  void trigger(SamplePlaybackSettings *settings)
  {
    if(! settings->reverse) { // if forward playback
      this->playback_position = (settings->offset * this->sample.size());
    }
    else { // if reverse playback
      this->playback_position = (( 1 - settings->offset) * this->sample.size());
    }

    this->playing = true;
  }

  void stop()
  {
    playing = false;
  }

	void step(float rack_sample_rate, SamplePlaybackSettings *settings)
	{
    if(playing && sample.loaded)
    {
      float step_amount = (sample.sample_rate / rack_sample_rate);

      // Step the playback position forward.
  		playback_position = playback_position + step_amount; // sample rate playback

      // Add more or less to the playback position based on the pitch setting
      playback_position = playback_position + (step_amount * ((settings->pitch * 2.0) - 1.0)); // 5 octave range

      // If settings loop is greater than 0 and the sample position is past the
      // selected loop length, then loop.  Note:  If loop is set to 1, then
      // the entire sample will loop.
      if(settings->loop > 0)
      {
        float loop_position = (settings->offset * sample.size()) + ((sample.size() - settings->offset) * settings->loop);
        if(playback_position >= loop_position) playback_position = (settings->offset * sample.size());
      }
      else
      {
        // If the playback position is past the playback length, end sample playback
        if(playback_position >= sample.size())
        {
           stop();
        }
      }
    }
	}

  void stepReverse(float rack_sample_rate, SamplePlaybackSettings *settings)
	{
    if(playing && sample.loaded)
    {
      float step_amount = sample.sample_rate / rack_sample_rate;

      // Step the playback position forward.
  		playback_position = playback_position - step_amount;
      playback_position = playback_position - (step_amount * ((settings->pitch * 2.0) - 1.0)); // 5 octave range

      // If the playback position is past the beginning, end or loop sample playback
      if(settings->loop > 0)
      {
        float playback_start = (1.0 - settings->offset) * sample.size();
        float loop_position = playback_start - (settings->loop * (sample.size() - playback_start));

        if(playback_position <= loop_position)
        {
          playback_position = playback_start;
        }
      }
      else
      {
        // If the playback position is past the playback length, end sample playback
        if(playback_position <= 0)
        {
           stop();
        }
      }
    }
	}

  void loadSample(std::string path)
  {
    sample.load(path);
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

}
