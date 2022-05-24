namespace groove_box
{

struct Track
{
  bool steps[NUMBER_OF_STEPS] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
  bool ratchet_patterns[8][7] = {
    {0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0},
    {0,1,0,1,0,1,0},
    {1,1,1,1,1,1,1},
    {0,1,0,0,1,0,0},
    {1,1,1,0,1,0,0},
    {0,0,0,1,1,1,1},
    {1,1,1,0,0,0,0}
  };
  unsigned int playback_position = 0;
  unsigned int length = NUMBER_OF_STEPS - 1;
  SamplePlaybackSettings sample_playback_settings[NUMBER_OF_STEPS]; // settings assigned to each step
  SamplePlaybackSettings settings; // currently used settings

  // Global track values set by the expandcer
  float track_pan = 0.0;
  float track_pitch = 0.0;
  float track_volume = 0.0;

  StereoPanSubModule stereo_pan_submodule;
  unsigned int ratchet_counter = 0;
  SamplePlayer *sample_player;

  // Variables to assist with fading out
  float fade_out_counter = 0;
  float fade_out_from = 0;
  float fading_out = false;

  // The "skipped" variable keep track of when a trigger has been skipped because
  // the "Percentage" funtion is non-zero and didn't fire on the current step.``
  bool skipped = false;

  Track()
  {
  }

  void setSamplePlayer(SamplePlayer *sample_player)
  {
    this->sample_player = sample_player;
  }

  void step()
  {
    playback_position = (playback_position + 1) % (length + 1);
    ratchet_counter = 0;
  }

  bool trigger()
  {
    fading_out = false;

    if((getProbability(playback_position) < 0.98) && (((float) rand()/RAND_MAX) > getProbability(playback_position)))
    {
      // Don't trigger
      skipped = true;
    }
    else
    {
      skipped = false;

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
        sample_player->trigger(&settings);

        return(true);
      }
    }
    return(false);
  }

  void fadeOut(float rack_sample_rate)
  {
    fade_out_counter = rack_sample_rate / 4.0;
    fade_out_from = fade_out_counter;
    fading_out = true;
  }

  //
  // Handle ratcheting
  //
  bool ratchety()
  {
    bool ratcheted = false;
    
    if (steps[playback_position] && (skipped == false))
    {
      unsigned int ratchet_pattern = settings.ratchet * 7;
      if(ratchet_patterns[ratchet_pattern][ratchet_counter])
      {
        sample_player->trigger(&settings);
        ratcheted = true;
      }
      if(++ratchet_counter >= 8) ratchet_counter = 0;
    }
    else
    {
      ratchet_counter = 0;
    }

    return(ratcheted);
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
    ratchet_counter = 0;
    fading_out = false;
  }

  void clear()
  {
    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      setValue(i, false);
    }
    this->length = NUMBER_OF_STEPS - 1;
    this->resetAllParameterLocks();
  }


  std::pair<float, float> getStereoOutput(unsigned int interpolation)
  {
    float left_output;
    float right_output;

    // Read sample output and return
    this->sample_player->getStereoOutput(&left_output, &right_output, interpolation);

    // settings.pan ranges from 0 to 1
    // track_pan ranges from -1 to 0
    float computed_pan = (settings.pan * 2.0) - 1.0; // convert settings.pan to range from -1 to 1
    computed_pan = clamp(computed_pan + track_pan, -1.0, 1.0);
    std::tie(left_output, right_output) = stereo_pan_submodule.process(left_output, right_output, computed_pan);

    left_output *= (settings.volume * 2);  // Range from 0 to 2 times normal volume
    right_output *= (settings.volume * 2);  // Range from 0 to 2 times normal volume

    if (fading_out)
    {
      fade_out_counter -= 1.0;

      if(fade_out_counter <= 0)
      {
        this->sample_player->stop();
        fading_out = false;
        left_output = 0;
        right_output = 0;
      }
      else
      {
        float fade_amount = fade_out_counter / fade_out_from;
        left_output *= fade_amount;
        right_output *= fade_amount;
        fade_out_counter--;
      }
    }

    return { left_output, right_output };
  }

  void incrementSamplePosition(float rack_sample_rate)
  {
    if(settings.reverse > .5)
    {
      this->sample_player->stepReverse(rack_sample_rate, &settings, track_pitch);
    }
    else
    {
      this->sample_player->step(rack_sample_rate, &settings, track_pitch);
    }
  }

  void copy(Track *src_track)
  {
    // First copy step data
    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      this->steps[i] = src_track->steps[i];
      this->sample_playback_settings[i].copy(& src_track->sample_playback_settings[i]);
    }

    this->settings.copy(&src_track->settings);

    // Copy single variables
    this->length = src_track->length;
    this->playback_position = src_track->playback_position;
    this->ratchet_counter = src_track->ratchet_counter;
    this->skipped = src_track->skipped;

    this->track_volume = src_track->track_volume;
    this->track_pan = src_track->track_pan;
    this->track_pitch = src_track->track_pitch;
  }

  float getLength()  {
    return(this->length);
  }

  void setLength(unsigned int length)  {
    this->length = length;
  }

  // Parameter locks
  // ============================================================================

  void resetAllParameterLocks()
  {
    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      setOffset(i, default_offset);
      setVolume(i, default_volume);
      setPan(i, default_pan);
      setPitch(i, default_pitch);
      setRatchet(i, default_ratchet);
      setOffset(i, default_offset);
      setProbability(i, default_probability);
      setLoop(i, default_loop);
      setReverse(i, default_reverse);
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
  // Probability
  //
  float getProbability(unsigned int step)  {
    return(this->sample_playback_settings[step].probability);
  }
  void setProbability(unsigned int step, float probability)  {
    this->sample_playback_settings[step].probability = probability;
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

  // Track pan
  float getTrackPan()  {
    return(this->track_pan);
  }
  void setTrackPan(float track_pan) {
    this->track_pan = track_pan;
  }

  // Track pitch
  float getTrackPitch()  {
    return(this->track_pitch);
  }
  void setTrackPitch(float track_pitch) {
    this->track_pitch = track_pitch;
  }

};

}
