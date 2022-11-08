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

  // DSP classes
  ADSR adsr;
  SimpleDelay delay;
  StereoFadeOut fade_out;
  rack::dsp::SlewLimiter volume_slew_limiter;
  rack::dsp::SlewLimiter pan_slew_limiter;
  float volume_slew_target = 0.0;
  float pan_slew_target = 0.0;

  // StereoPanSubModule stereo_pan_submodule;
  StereoPan stereo_pan;
  unsigned int ratchet_counter = 0;
  SamplePlayer *sample_player;


  // The "skipped" variable keep track of when a trigger has been skipped because
  // the "Percentage" funtion is non-zero and didn't fire on the current step.``
  bool skipped = false;

  // Transitory variables
  int shift_starting_column = 0;

  // Sample rate
  float rack_sample_rate = APP->engine->getSampleRate();

  Track()
  {
    adsr.setAttackRate(0);
    adsr.setDecayRate(0); // no decay stage for this module
    adsr.setReleaseRate(1 * APP->engine->getSampleRate()); // 1 second
    adsr.setSustainLevel(1.0);

    volume_slew_limiter.setRiseFall(900.0f, 900.0f); // 900 works.  I want the highest number possible for the shortest slew
    pan_slew_limiter.setRiseFall(900.0f, 900.0f);

    delay.setBufferSize(APP->engine->getSampleRate() / 30.0);
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
    fade_out.reset();

    if((getProbability(playback_position) < 0.98) && (((float) rand()/(float) RAND_MAX) > getProbability(playback_position)))
    {
      // Don't trigger
      skipped = true;
    }
    else
    {
      skipped = false;

      if (steps[playback_position])
      {
        // It's necessary to slew the volume and pan, otherwise these will
        // introduce a pop or click when modulated between distant values
        volume_slew_target = getVolume(playback_position);
        pan_slew_target = getPan(playback_position);

        // Change all other settings immediately when triggered
        settings.pitch = getPitch(playback_position);
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
        sample_player->trigger(settings.sample_start, settings.reverse);

        return(true);
      }
    }
    return(false);
  }

  void fadeOut()
  {
    fade_out.trigger();
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
        sample_player->trigger(settings.sample_start, settings.reverse);
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
    fade_out.reset();
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

  void copyStep(unsigned int copy_from_index, unsigned int copy_to_index)
  {
    if(copy_from_index != copy_to_index)
    {
      sample_playback_settings[copy_to_index].copy(& sample_playback_settings[copy_from_index]);
      this->steps[copy_to_index] = this->steps[copy_from_index];
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

    // -===== ADSR Processing =====-
    //
    adsr.setAttackRate(settings.attack *  APP->engine->getSampleRate());
    adsr.setReleaseRate(settings.release * maximum_release_time * APP->engine->getSampleRate());

    float adsr_value = adsr.process();

    // When the ADSR reaches the sustain state, then switch to the release
    // state.  Only do this when the release is less than max release, otherwise
    // sustain until the nex time the track is triggered.
    //
    // * Reminder: settings.release ranges from 0.0 to 1.0
    //
    if(adsr.getState() == ADSR::env_sustain && settings.release < 1.0) adsr.gate(false);

    // -===== Slew Limiter Processing =====-
    //
    settings.volume = volume_slew_limiter.process(APP->engine->getSampleTime(), volume_slew_target);
    settings.pan = pan_slew_limiter.process(APP->engine->getSampleTime(), pan_slew_target);

    // Read sample output
    this->sample_player->getStereoOutput(&left_output, &right_output, interpolation);

    // Apply pan parameters
    //
    // settings.pan ranges from 0 to 1
    // track_pan ranges from -1 to 0
    float computed_pan = (settings.pan * 2.0) - 1.0; // convert settings.pan to range from -1 to 1
    computed_pan = clamp(computed_pan + track_pan, -1.0, 1.0);
    stereo_pan.process(&left_output, &right_output, computed_pan);

    // Apply volume parameters
    left_output *= (settings.volume * 2);  // Range from 0 to 2 times normal volume
    right_output *= (settings.volume * 2);  // Range from 0 to 2 times normal volume

    // Process fade out at 1/10th of a second.
    //
    // The fade_out.process() method will pass through the audio untouched if
    // there's no fade in progress.  It will return TRUE on the event of a fade
    // having been completed.
    //
    // Why fade?  At the moment, the only reason to fade_out is when a track
    // is muted by the expander.
    //
    if (fade_out.process(&left_output, &right_output, 10.0 * APP->engine->getSampleTime()))
    {
      // If this line has been reached, it means the a fade out has just completed
      // If so, stop the sample player
      this->sample_player->stop();
    }

    // Apply ADSR to volume
    left_output *= adsr_value;
    right_output *= adsr_value;

    // If the delay mix is above 0 for this track, then compute the delay and
    // output it.  This if statement is an attempt to trim down CPU usage when
    // the delay is effectively turned off.
    if(settings.delay_mix > 0)
    {
      float delay_output_left = 0.0;
      float delay_output_right = 0.0;

      // Apply delay
      delay.setMix(settings.delay_mix);
      delay.setBufferSize(settings.delay_length * (APP->engine->getSampleRate() / 4));
      delay.setFeedback(settings.delay_feedback);
      delay.process(left_output, right_output, delay_output_left, delay_output_right);

      // Return audio with the delay applied
      return { delay_output_left, delay_output_right };
    }
    else
    {
      // In this case, the delay is turned off, so skip all of the delay computations
      // and simply return the output computed up to now.
      return { left_output, right_output };
    }

  }

  void incrementSamplePosition()
  {
    
    float summed_pitch = clamp(settings.pitch + track_pitch, 0.0, 1.0);

    if(settings.reverse > .5)
    {
      // -2.0 to 2.0 is a two octave range in either direction (4 octives total)
      this->sample_player->stepReverse(rescale(summed_pitch, 0.0, 1.0, -2.0, 2.0), settings.sample_start, settings.sample_end, settings.loop);
    }
    else
    {
      this->sample_player->step(rescale(summed_pitch, 0.0, 1.0, -2.0, 2.0), settings.sample_start, settings.sample_end, settings.loop);
    }
  }

  void updateRackSampleRate()
  {
    rack_sample_rate = APP->engine->getSampleRate();
    this->sample_player->updateSampleRate();
  }

  bool isFadingOut()
  {
    return(fade_out.fading_out);
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

  void randomizeSteps()
  {
    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      steps[i] = (rand() > (RAND_MAX / 2));
    }
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
