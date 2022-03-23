namespace scalar_110
{

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
    if((playing == false) || (sample_position >= this->sample.size()) || (sample.loaded == false)) return { 0,0 };
    float left; float right;
    std::tie(left, right) = this->sample.read(sample_position);
		return { left, right };
	}

  void trigger(float offset_percentage)
  {
    playback_position = (offset_percentage * this->sample.size());
    playing = true;
  }

  void stop()
  {
    playing = false;
  }

	void step(float rack_sample_rate, float pitch_cv_input)
	{
    if(playing && sample.loaded)
    {
      float step_amount = sample.sample_rate / rack_sample_rate;

      // Step the playback position forward.
      // Need to revisit this and get it right.
  		playback_position = playback_position + step_amount;
      playback_position = playback_position + (step_amount * (pow(2,pitch_cv_input * 5)  - 1)); // 5 octave range

      // voltage of 0 should playback at normal rate
      // voltage of 1 should playback at 2^1 == twice as fast
      // voltage of 2 should playback at 2^2 == four times as fast

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

}
