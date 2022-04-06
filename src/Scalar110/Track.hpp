namespace scalar_110
{

struct Track
{
  bool steps[NUMBER_OF_STEPS] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
  bool ratchet_patterns[6][7] = {
    {0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0},
    {0,1,0,1,0,1,0},
    {1,1,1,1,1,1,1},
    {0,1,0,0,1,0,0},
    {1,1,1,0,1,0,0},
  };
  unsigned int playback_position = 0;
  SamplePlaybackSettings sample_playback_settings[NUMBER_OF_STEPS]; // settings assigned to each step
  SamplePlaybackSettings settings; // currently used settings

  StereoPanSubModule stereo_pan_submodule;
  unsigned int ratchet_counter = 0;
  SamplePlayer sample_player;

  Track()
  {
  }

  void step()
  {
    playback_position = (playback_position + 1) % NUMBER_OF_STEPS;
    ratchet_counter = 0;

    if (steps[playback_position])
    {
      // Copy the settings from sample_playback_settings into settings
      settings.volume = getVolume(playback_position);
      settings.pitch = getPitch(playback_position);
      settings.pan = getPan(playback_position);
      settings.ratchet = getRatchet(playback_position);
      settings.offset = getOffset(playback_position);
      settings.reverse = getReverse(playback_position);
      settings.loop = getLoop(playback_position);

      // trigger sample playback
      sample_player.trigger(&settings);
    }
  }

  void ratchety()
  {
    if (steps[playback_position])
    {
      unsigned int ratchet_pattern = settings.ratchet * 5;
      if(ratchet_patterns[ratchet_pattern][ratchet_counter]) sample_player.trigger(&settings);
      if(++ratchet_counter >= 8) ratchet_counter = 0;
    }
    else
    {
      ratchet_counter = 0;
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

    float centered_pan = (settings.pan * 2.0) - 1.0;
    std::tie(left_output, right_output) = stereo_pan_submodule.process(left_output, right_output, centered_pan);

    left_output *= (settings.volume * 2);  // Range from 0 to 2 times normal volume
    right_output *= (settings.volume * 2);  // Range from 0 to 2 times normal volume

    return { left_output, right_output };
  }

  void incrementSamplePosition(float rack_sample_rate)
  {
    if(settings.reverse > .5)
    {
      this->sample_player.stepReverse(rack_sample_rate, &settings);
    }
    else
    {
      this->sample_player.step(rack_sample_rate, &settings);
    }
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

  //
  // Reverse
  //
  float getReverse(unsigned int step)  {
    return(this->sample_playback_settings[step].reverse);
  }
  void setReverse(unsigned int step, float reverse)  {
    this->sample_playback_settings[step].reverse = reverse;
  }

  //
  // Loop
  //
  float getLoop(unsigned int step)  {
    return(this->sample_playback_settings[step].loop);
  }
  void setLoop(unsigned int step, float loop)  {
    this->sample_playback_settings[step].loop = loop;
  }

};

}
