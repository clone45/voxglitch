namespace scalar_110
{

struct Track
{
  bool steps[NUMBER_OF_STEPS] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
  unsigned int playback_position = 0;
  SamplePlaybackSettings sample_playback_settings[NUMBER_OF_STEPS];
  float volume = .5;
  float pan = .5;
  float pitch = .5;
  StereoPanSubModule stereo_pan_submodule;
  unsigned int ratchet_counter = 0;
  SamplePlayer sample_player;

  Track()
  {
    volume = getVolume(playback_position);
  }

  void step()
  {
    playback_position = (playback_position + 1) % NUMBER_OF_STEPS;
    ratchet_counter = 0;

    //
    // TODO: possibly send over a pointer to ALL of the step parameters
    //       and copy them locally in the sample_player?

    if (steps[playback_position])
    {
      // trigger sample playback
      volume = getVolume(playback_position);
      pitch = getPitch(playback_position);
      pan = getPan(playback_position);

      sample_player.trigger(this->sample_playback_settings[playback_position].offset);
    }
  }

  void ratchet()
  {
    if (steps[playback_position])
    {
      unsigned int ratchet = getRatchet(playback_position) * 8;
      if(ratchet > 0)
      {
        ratchet = 8 - ratchet;

        if(ratchet_counter >= ratchet)
        {
            // ratchet!
            sample_player.trigger(this->sample_playback_settings[playback_position].offset);

            // Reset ratchet counter
            ratchet_counter = 0;
        }
        ratchet_counter++;
      }
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

    float centered_pan = (this->pan * 2.0) - 1.0;
    std::tie(left_output, right_output) = stereo_pan_submodule.process(left_output, right_output, centered_pan);

    left_output *= (volume * 2);  // Range from 0 to 2 times normal volume
    right_output *= (volume * 2);  // Range from 0 to 2 times normal volume

    return { left_output, right_output };
  }

  void incrementSamplePosition(float rack_sample_rate)
  {
    this->sample_player.step(rack_sample_rate, pitch);
  }

  //
  // Offset
  //
  float getOffset(unsigned int step) {
    return(this->sample_playback_settings[step].offset);
  }
  void setOffset(unsigned int step, float offset) {
    this->sample_playback_settings[step].offset = offset;
  }

  //
  // Volume
  //
  float getVolume(unsigned int step) {
    return(this->sample_playback_settings[step].volume);
  }
  void setVolume(unsigned int step, float volume) {
    this->sample_playback_settings[step].volume = volume;
  }

  //
  // Pitch
  //
  float getPitch(unsigned int step)  {
    return(this->sample_playback_settings[step].pitch);
  }
  void setPitch(unsigned int step, float pitch)  {
    this->sample_playback_settings[step].pitch = pitch;
  }

  //
  // Pan
  //
  float getPan(unsigned int step)  {
    return(this->sample_playback_settings[step].pan);
  }
  void setPan(unsigned int step, float pan)  {
    this->sample_playback_settings[step].pan = pan;
  }

  //
  // Ratchet
  //
  float getRatchet(unsigned int step)  {
    return(this->sample_playback_settings[step].ratchet);
  }
  void setRatchet(unsigned int step, float ratchet)  {
    this->sample_playback_settings[step].ratchet = ratchet;
  }

};

}
