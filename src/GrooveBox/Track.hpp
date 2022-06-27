// Thought:
// When calling Trigger, pass in the global modifiers, such as "sample_position_snap",
// to apply to the sample start position and sample end position.

namespace groove_box
{

struct Track
{
  bool steps[NUMBER_OF_STEPS] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};


  unsigned int playback_position = 0;
  unsigned int range_end = NUMBER_OF_STEPS - 1; // was length
  unsigned int range_start = 0;
  SamplePlaybackSettings sample_playback_settings[NUMBER_OF_STEPS]; // settings assigned to each step
  SamplePlaybackSettings settings; // currently used settings

  // Global track values set by the expandcer
  float track_pan = 0.0;
  float track_pitch = 0.0;
  float track_volume = 0.0;

  ADSR adsr;
  SimpleDelay delay;

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

  // Transitory variables
  int shift_starting_column = 0;

  Track()
  {
    adsr.setAttackRate(0);
    adsr.setDecayRate(0); // no decay stage for this module
    adsr.setReleaseRate(1 * rack::settings::sampleRate); // 1 second
    adsr.setSustainLevel(1.0);

    delay.setBufferSize(rack::settings::sampleRate / 30.0);
  }

  void setSamplePlayer(SamplePlayer *sample_player)
  {
    this->sample_player = sample_player;
  }

  void step()
  {
    playback_position = playback_position + 1;
    if(playback_position > range_end) playback_position = range_start;
    ratchet_counter = 0;
  }

  bool trigger(unsigned int sample_position_snap_value)
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
        settings.sample_start = getSampleStart(playback_position);
        settings.sample_end = getSampleEnd(playback_position);
        settings.reverse = getReverse(playback_position);
        settings.loop = getLoop(playback_position);
        settings.attack = getAttack(playback_position);
        settings.release = getRelease(playback_position);
        settings.delay_mix = getDelayMix(playback_position);
        settings.delay_length = getDelayLength(playback_position);
        settings.delay_feedback = getDelayFeedback(playback_position);

        // If the sample start settings is set and snap is on, then quantize the sample start position.
        if(sample_position_snap_value > 0 && settings.sample_start > 0)
        {
          // settings.sample_start ranges from 0 to 1
          // This next line sets quantized_sample_start to an integer between 0 and sample_position_snap
          float quantized_sample_start = settings.sample_start * (float) sample_position_snap_value;
          quantized_sample_start = std::floor(quantized_sample_start);
          settings.sample_start = quantized_sample_start / (float) sample_position_snap_value;
        }

        // Trigger the ADSR
        adsr.gate(true);

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
      unsigned int ratchet_pattern = settings.ratchet * (NUMBER_OF_RATCHET_PATTERNS - 1);
      if(ratchet_patterns[ratchet_pattern][ratchet_counter])
      {
        sample_player->trigger(&settings);
        adsr.gate(true); // retrigger the ADSR
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
    playback_position = range_start;
    ratchet_counter = 0;
    fading_out = false;
  }

  void clear()
  {
    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      setValue(i, false);
    }
    this->range_end = NUMBER_OF_STEPS - 1;
    this->range_start = 0;
    this->resetAllParameterLocks();
  }

  void shift(unsigned int amount)
  {
    if(amount > 0)
    {
      // Create a copy of all of the playback settings for this track
      SamplePlaybackSettings temp_settings[NUMBER_OF_STEPS];
      bool temp_steps[NUMBER_OF_STEPS];

      for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
      {
        temp_settings[i].copy(& sample_playback_settings[i]);
        temp_steps[i] = this->steps[i];
      }

      // Now copy the track information back into the shifted location
      for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
      {
        unsigned int copy_from_index = (i + amount) % NUMBER_OF_STEPS;
        sample_playback_settings[i].copy(& temp_settings[copy_from_index]);
        this->steps[i] = temp_steps[copy_from_index];
      }
    }
  }

  void initialize()
  {
    this->clear();
    this->sample_player->initialize();
  }

  std::pair<float, float> getStereoOutput(unsigned int interpolation)
  {
    float left_output;
    float right_output;

    adsr.setAttackRate(settings.attack *  rack::settings::sampleRate);
    adsr.setReleaseRate(settings.release * maximum_release_time * rack::settings::sampleRate);

    float adsr_value = adsr.process();

    // When the ADSR reaches the sustain state, then switch to the release
    // state.  Only do this when the release is less than max release, otherwise
    // sustain until the nex time the track is triggered.
    //
    // * Reminder: settings.release ranges from 0.0 to 1.0
    //
    if(adsr.getState() == ADSR::env_sustain && settings.release < 1.0) adsr.gate(false);

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

    // Apply ADSR to volume
    left_output *= adsr_value;
    right_output *= adsr_value;

    // Apply reverb
    // float reverb_input = (left_output + right_output) / 2;
    // reverb.process(reverb_input, left_output, right_output);

    // Apply delay
    float delay_output_left = 0.0;
    float delay_output_right = 0.0;

    delay.setMix(settings.delay_mix);
    delay.setBufferSize(settings.delay_length * (rack::settings::sampleRate / 4));
    delay.setFeedback(settings.delay_feedback);
    delay.process(left_output, right_output, delay_output_left, delay_output_right);

    return { delay_output_left, delay_output_right };
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
    this->range_end = src_track->range_end;
    this->range_start = src_track->range_start;
    this->playback_position = src_track->playback_position;
    this->ratchet_counter = src_track->ratchet_counter;
    this->skipped = src_track->skipped;

    this->track_volume = src_track->track_volume;
    this->track_pan = src_track->track_pan;
    this->track_pitch = src_track->track_pitch;
  }

  float getRangeStart()  {
    return(this->range_start);
  }

  void setRangeStart(unsigned int range_start)  {
    if(playback_position < range_start) playback_position = range_start;
    this->range_start = range_start;
  }

  float getRangeEnd()  {
    return(this->range_end);
  }

  void setRangeEnd(unsigned int range_end)  {
    this->range_end = range_end;
  }



  // Parameter locks
  // ============================================================================

  void resetAllParameterLocks()
  {
    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      setVolume(i, default_volume);
      setPan(i, default_pan);
      setPitch(i, default_pitch);
      setRatchet(i, default_ratchet);
      setSampleStart(i, default_sample_start);
      setSampleEnd(i, default_sample_end);
      setProbability(i, default_probability);
      setLoop(i, default_loop);
      setReverse(i, default_reverse);
      setAttack(i, default_attack);
      setRelease(i, default_release);
      setDelayMix(i, default_delay_mix);
      setDelayLength(i, default_delay_length);
      setDelayFeedback(i, default_delay_feedback);
    }
  }

  //
  // Sample Start
  //
  float getSampleStart(unsigned int step) {  // This could apply the snap here
    return(this->sample_playback_settings[step].sample_start);
  }
  void setSampleStart(unsigned int step, float sample_start) {
    this->sample_playback_settings[step].sample_start = sample_start;
  }

  //
  // Sample End
  //
  float getSampleEnd(unsigned int step) {  // This could apply the snap here
    return(this->sample_playback_settings[step].sample_end);
  }
  void setSampleEnd(unsigned int step, float sample_end) {
    this->sample_playback_settings[step].sample_end = sample_end;
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

  //
  // Attack
  //
  float getAttack(unsigned int step)  {
    return(this->sample_playback_settings[step].attack);
  }
  void setAttack(unsigned int step, float attack)  {
    this->sample_playback_settings[step].attack = attack;
  }

  //
  // Release
  //
  float getRelease(unsigned int step)  {
    return(this->sample_playback_settings[step].release);
  }
  void setRelease(unsigned int step, float release)  {
    this->sample_playback_settings[step].release = release;
  }

  //
  // Delay Mix
  //
  float getDelayMix(unsigned int step)  {
    return(this->sample_playback_settings[step].delay_mix);
  }
  void setDelayMix(unsigned int step, float delay_mix)  {
    this->sample_playback_settings[step].delay_mix = delay_mix;
  }

  //
  // Delay Length
  //
  float getDelayLength(unsigned int step)  {
    return(this->sample_playback_settings[step].delay_length);
  }
  void setDelayLength(unsigned int step, float delay_length)  {
    this->sample_playback_settings[step].delay_length = delay_length;
  }

  //
  // Delay Feedback
  //
  float getDelayFeedback(unsigned int step)  {
    return(this->sample_playback_settings[step].delay_feedback);
  }
  void setDelayFeedback(unsigned int step, float delay_feedback)  {
    this->sample_playback_settings[step].delay_feedback = delay_feedback;
  }

  // =-==================================================================

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
