namespace scalar_110
{

struct Track
{
  bool steps[NUMBER_OF_STEPS] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
  unsigned int playback_position = 0;
  SamplePlaybackSettings sample_playback_settings[NUMBER_OF_STEPS];

  std::string loaded_filename = "";
  SamplePlayer sample_player;

  Track()
  {

  }

  void step()
  {
    playback_position = (playback_position + 1) % NUMBER_OF_STEPS;

    //
    // TODO: possibly send over a pointer to ALL of the step parameters
    //       and copy them locally in the sample_player?

    if (steps[playback_position])
    {
      // trigger sample playback
      sample_player.trigger(this->sample_playback_settings[playback_position].offset);
    }
  }

  unsigned int getPosition()
  {
    return(playback_position);
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

  std::pair<float, float> getStereoOutput()
  {
    float left_output;
    float right_output;

    // Read sample output and return
    std::tie(left_output, right_output) = this->sample_player.getStereoOutput();
    return { left_output, right_output };
  }


  float getOffset(unsigned int selected_step)
  {
    return(this->sample_playback_settings[selected_step].offset);
  }

  void setOffset(unsigned int selected_step, float offset)
  {
    this->sample_playback_settings[selected_step].offset = offset;
  }

  float getVolume(unsigned int selected_step)
  {
    return(this->sample_playback_settings[selected_step].volume);
  }

  void setVolume(unsigned int selected_step, float volume)
  {
    this->sample_playback_settings[selected_step].volume = volume;
  }

};

}
