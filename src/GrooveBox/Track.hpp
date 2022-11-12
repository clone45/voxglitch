// Thought:
// When calling Trigger, pass in the global modifiers, such as "sample_position_snap",
// to apply to the sample start position and sample end position.

namespace groove_box
{

  struct Track
  {
    bool steps[NUMBER_OF_STEPS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    unsigned int playback_position = 0;
    unsigned int range_end = NUMBER_OF_STEPS - 1; // was length
    unsigned int range_start = 0;
    SamplePlaybackSettings sample_playback_settings[NUMBER_OF_STEPS]; // settings assigned to each step
    SamplePlaybackSettings settings;                                  // currently used settings

    // Global track values set by the expandcer
    float track_pan = 0.0;
    float track_pitch = 0.0;
    float track_volume = 0.0;

    // DSP classes
    ADSR adsr;
    SimpleDelay delay;
    StereoFadeOut fade_out;
    Filter filter;
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
      adsr.setDecayRate(0);                                  // no decay stage for this module
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
      if (playback_position > range_end)
        playback_position = range_start;
      ratchet_counter = 0;
    }

    bool trigger(unsigned int sample_position_snap_value)
    {
      fade_out.reset();

      float probability = getParameter(PROBABILITY, playback_position);

      if ((probability < 0.98) && (((float)rand() / (float)RAND_MAX) > probability))
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
          // volume_slew_target = getVolume(playback_position);
          // pan_slew_target = getPan(playback_position);

          volume_slew_target = getParameter(VOLUME, playback_position);
          pan_slew_target = getParameter(PAN, playback_position);

          for (unsigned int parameter_number = 0; parameter_number < NUMBER_OF_FUNCTIONS; parameter_number++)
          {
            settings.parameters[parameter_number] = getParameter(parameter_number, playback_position);
          }

          // If the sample start settings is set and snap is on, then quantize the sample start position.
          float sample_start = settings.parameters[SAMPLE_START];

          if (sample_position_snap_value > 0 && sample_start > 0)
          {
            // settings.sample_start ranges from 0 to 1
            // This next line sets quantized_sample_start to an integer between 0 and sample_position_snap
            // float quantized_sample_start = settings.sample_start * (float)sample_position_snap_value;
            float quantized_sample_start = sample_start * (float)sample_position_snap_value;
            quantized_sample_start = std::floor(quantized_sample_start);
            settings.parameters[SAMPLE_START] = quantized_sample_start / (float)sample_position_snap_value;
          }

          // Trigger the ADSR
          adsr.gate(true);

          // trigger sample playback
          sample_player->trigger(sample_start, settings.parameters[REVERSE]);

          return (true);
        }
      }
      return (false);
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
        unsigned int ratchet_pattern = settings.parameters[RATCHET] * (NUMBER_OF_RATCHET_PATTERNS - 1);
        if (ratchet_patterns[ratchet_pattern][ratchet_counter])
        {
          sample_player->trigger(settings.parameters[SAMPLE_START], settings.parameters[REVERSE]);
          adsr.gate(true); // retrigger the ADSR
          ratcheted = true;
        }
        if (++ratchet_counter >= 8)
          ratchet_counter = 0;
      }
      else
      {
        ratchet_counter = 0;
      }

      return (ratcheted);
    }

    unsigned int getPosition()
    {
      return (playback_position);
    }

    void setPosition(unsigned int playback_position)
    {
      if (playback_position < NUMBER_OF_STEPS)
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
      return (steps[i]);
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
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
      {
        setValue(i, false);
      }
      this->range_end = NUMBER_OF_STEPS - 1;
      this->range_start = 0;
      this->resetAllParameterLocks();
    }

    void clearSteps()
    {
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
      {
        setValue(i, false);
      }
    }
    /*
    void randomizeParameter()
    {
      for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
      {
        setValue(i, false);
      }
    }
    */

    void shift(unsigned int amount)
    {
      if (amount > 0)
      {
        // Create a copy of all of the playback settings for this track
        SamplePlaybackSettings temp_settings[NUMBER_OF_STEPS];
        bool temp_steps[NUMBER_OF_STEPS];

        for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
        {
          temp_settings[i].copy(&sample_playback_settings[i]);
          temp_steps[i] = this->steps[i];
        }

        // Now copy the track information back into the shifted location
        for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
        {
          unsigned int copy_from_index = (i + amount) % NUMBER_OF_STEPS;
          sample_playback_settings[i].copy(&temp_settings[copy_from_index]);
          this->steps[i] = temp_steps[copy_from_index];
        }
      }
    }

    void copyStep(unsigned int copy_from_index, unsigned int copy_to_index)
    {
      if (copy_from_index != copy_to_index)
      {
        sample_playback_settings[copy_to_index].copy(&sample_playback_settings[copy_from_index]);
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

      // -===== Slew Limiter Processing =====-
      settings.setParameter(VOLUME, volume_slew_limiter.process(APP->engine->getSampleTime(), volume_slew_target));
      settings.setParameter(PAN, pan_slew_limiter.process(APP->engine->getSampleTime(), pan_slew_target));

      // Mostly cosmetic: Load up the settings into easily readable variables
      // The "settings" structure is populated when the track is stepped.  It
      // contains of snapshot of the settings related to the active step.

      float attack = settings.getParameter(ATTACK);
      float release = settings.getParameter(RELEASE);
      float volume = settings.getParameter(VOLUME);      
      float pan = settings.getParameter(PAN);
      float filter_cutoff = settings.getParameter(FILTER_CUTOFF);
      float filter_resonance = settings.getParameter(FILTER_RESONANCE);
      float delay_mix = settings.getParameter(DELAY_MIX);
      float delay_length = settings.getParameter(DELAY_LENGTH);
      float delay_feedback = settings.getParameter(DELAY_FEEDBACK);

      // -===== ADSR Processing =====-
      //
      adsr.setAttackRate(attack * APP->engine->getSampleRate());
      adsr.setReleaseRate(release * maximum_release_time * APP->engine->getSampleRate());

      float adsr_value = adsr.process();

      // When the ADSR reaches the sustain state, then switch to the release
      // state.  Only do this when the release is less than max release, otherwise
      // sustain until the nex time the track is triggered.
      //
      // * Reminder: settings.getParameter(RELEASE) ranges from 0.0 to 1.0
      //
      if (adsr.getState() == ADSR::env_sustain && release < 1.0)
        adsr.gate(false);

      // Read sample output
      this->sample_player->getStereoOutput(&left_output, &right_output, interpolation);

      // Apply pan parameters
      //
      // settings.pan ranges from 0 to 1
      // track_pan ranges from -1 to 0
      float computed_pan = (pan * 2.0) - 1.0; // convert settings.pan to range from -1 to 1
      computed_pan = clamp(computed_pan + track_pan, -1.0, 1.0);
      stereo_pan.process(&left_output, &right_output, computed_pan);

      // Apply volume parameters
      left_output *= (volume * 2);  // Range from 0 to 2 times normal volume
      right_output *= (volume * 2); // Range from 0 to 2 times normal volume

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

      // Apply filter
      filter.setCutoffAndResonance(filter_cutoff, filter_resonance);
      filter.process(&left_output, &right_output);

      // If the delay mix is above 0 for this track, then compute the delay and
      // output it.  This if statement is an attempt to trim down CPU usage when
      // the delay is effectively turned off.
      if (delay_mix > 0)
      {
        float delay_output_left = 0.0;
        float delay_output_right = 0.0;

        // Apply delay
        delay.setMix(delay_mix);
        delay.setBufferSize(delay_length * (APP->engine->getSampleRate() / 4));
        delay.setFeedback(delay_feedback);
        delay.process(left_output, right_output, delay_output_left, delay_output_right);

        // Return audio with the delay applied
        return {delay_output_left, delay_output_right};
      }
      else
      {
        // In this case, the delay is turned off, so skip all of the delay computations
        // and simply return the output computed up to now.
        return {left_output, right_output};
      }
    }

    void incrementSamplePosition()
    {
      float pitch = settings.getParameter(PITCH);
      float reverse = settings.getParameter(REVERSE);
      float sample_start = settings.getParameter(SAMPLE_START);
      float sample_end = settings.getParameter(SAMPLE_END);
      float loop = settings.getParameter(LOOP);

      float summed_pitch = clamp(pitch + track_pitch, 0.0, 1.0);

      if (reverse > .5)
      {
        // -2.0 to 2.0 is a two octave range in either direction (4 octives total)
        this->sample_player->stepReverse(rescale(summed_pitch, 0.0, 1.0, -2.0, 2.0), sample_start, sample_end, loop);
      }
      else
      {
        this->sample_player->step(rescale(summed_pitch, 0.0, 1.0, -2.0, 2.0), sample_start, sample_end, loop);
      }
    }

    void updateRackSampleRate()
    {
      rack_sample_rate = APP->engine->getSampleRate();
      this->sample_player->updateSampleRate();
    }

    bool isFadingOut()
    {
      return (fade_out.fading_out);
    }

    void copy(Track *src_track)
    {
      // First copy step data
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
      {
        this->steps[i] = src_track->steps[i];
        this->sample_playback_settings[i].copy(&src_track->sample_playback_settings[i]);
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
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
      {
        steps[i] = (rand() > (RAND_MAX / 2));
      }
    }

    float getRangeStart()
    {
      return (this->range_start);
    }

    void setRangeStart(unsigned int range_start)
    {
      if (playback_position < range_start) playback_position = range_start;
      this->range_start = range_start;
    }

    float getRangeEnd()
    {
      return (this->range_end);
    }

    void setRangeEnd(unsigned int range_end)
    {
      this->range_end = range_end;
    }

    // Parameter locks
    // ============================================================================
    void resetAllParameterLocks()
    {
      for (unsigned int step = 0; step < NUMBER_OF_STEPS; step++)
      {
        for (unsigned int parameter_number = 0; parameter_number < NUMBER_OF_FUNCTIONS; parameter_number++)
        {
          setParameter(parameter_number, step, default_parameter_values[parameter_number]);
        }
      }
    }

    // Be careful here.  setParameter and getParameter are helper functions.  There's 
    // also similar methods in SamplePlaybackSettings.hpp which are specific
    // to a track's current step's parameters.

    float getParameter(unsigned int parameter_number, unsigned int step)
    {
      return (this->sample_playback_settings[step].getParameter(parameter_number)); // [parameter_number])
    }


    void setParameter(unsigned int parameter_number, unsigned int step, float value)
    {
      // this->sample_playback_settings[step]->parameters[parameter_number] = value;
      this->sample_playback_settings[step].setParameter(parameter_number, value);
    }

    // "track_pan" is the global pan applied by the expander module
    float getTrackPan()  {
      return(this->track_pan);
    }
    void setTrackPan(float track_pan) {
      this->track_pan = track_pan;
    }

    // "track_pitch" is the global pitch applied by the expander module
    float getTrackPitch()  {
      return(this->track_pitch);
    }
    void setTrackPitch(float track_pitch) {
      this->track_pitch = track_pitch;
    }

  };

}
