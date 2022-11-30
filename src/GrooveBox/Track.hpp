// Thought:
// When calling Trigger, pass in the global modifiers, such as "sample_position_snap",
// to apply to the sample start position and sample end position.

namespace groove_box
{

  struct Track
  {
    TrackModel m;

    // DSP classes
    ADSR adsr;
    SimpleDelay *delay;
    StereoFadeOut fade_out;
    Filter filter;
    rack::dsp::SlewLimiter *volume_slew_limiter;
    rack::dsp::SlewLimiter *pan_slew_limiter;
    rack::dsp::SlewLimiter *filter_cutoff_slew_limiter;
    rack::dsp::SlewLimiter *filter_resonance_slew_limiter;

    StereoPan stereo_pan;

    // Notes on the next line of code: 
    SamplePlayer *sample_player;
    //
    // Each memory slot has 8 tracks, and there are 16 memory slots.  That
    // means that there are ... (computing) ... 128 tracks total.  However,
    // the module only contains 8 sample players.
    // Memory slot #0, track #0 points to the SamplePlayer[0]
    // Memory slot #1, track #0 points to the SamplePlayer[0]
    // Memory slot #2, track #0 points to the SamplePlayer[0]
    // and so on...
    //
    // In other words, sample players are shared between tracks. I didn't think
    // it would be feasible to have 128 different sample players, but I might
    // rethink this in the future.  As long as the sample waveform data isn't
    // duplicated, the overhead wouldn't be too bad.
    

    Track()
    {
      adsr.setAttackRate(0);
      adsr.setDecayRate(0);                                  // no decay stage for this module
      adsr.setReleaseRate(1 * APP->engine->getSampleRate()); // 1 second
      adsr.setSustainLevel(1.0);
    }

    void setSamplePlayer(SamplePlayer *sample_player)
    {
      this->sample_player = sample_player;
    }

    void setVolumeSlewLimiter(rack::dsp::SlewLimiter *slew_limiter)
    {
      volume_slew_limiter = slew_limiter;
    }

    void setPanSlewLimiter(rack::dsp::SlewLimiter *slew_limiter)
    {
      pan_slew_limiter = slew_limiter;
    }

    void setFilterCutoffSlewLimiter(rack::dsp::SlewLimiter *slew_limiter)
    {
      filter_cutoff_slew_limiter = slew_limiter;
    }

    void setFilterResonanceSlewLimiter(rack::dsp::SlewLimiter *slew_limiter)
    {
      filter_resonance_slew_limiter = slew_limiter;
    }

    void setDelayDsp(SimpleDelay *delay_dsp)
    {
      delay = delay_dsp;
    }

    void step()
    {
      m.playback_position = m.playback_position + 1;
      if (m.playback_position > m.range_end)
        m.playback_position = m.range_start;
      m.ratchet_counter = 0;
    }

    bool trigger(unsigned int sample_position_snap_value)
    {
      fade_out.reset();

      float probability = getParameter(PROBABILITY, m.playback_position);

      if ((probability < 0.98) && (((float)rand() / (float)RAND_MAX) > probability))
      {
        // Don't trigger
        m.skipped = true;
      }
      else
      {
        m.skipped = false;

        if (m.steps[m.playback_position])
        {
          // It's necessary to slew the volume and pan, otherwise these will
          // introduce a pop or click when modulated between distant values
          // I'm doing the same for filter cutoff and resonance, just out of paranoia.

          // m.volume_slew_target = getParameter(VOLUME, m.playback_position);
          // m.pan_slew_target = getParameter(PAN, m.playback_position);
          // m.filter_cutoff_slew_target = getParameter(FILTER_CUTOFF, m.playback_position);
          // m.filter_resonance_slew_target = getParameter(FILTER_RESONANCE, m.playback_position);

          // TODO: It's likely that right now the slew limiters aren't effective
          // because this next block of code sets the local parameter locks
          // equal to the playback_position's parameter locks, which leaves
          // nothing for the slew limiters to do.
          for (unsigned int parameter_number = 0; parameter_number < NUMBER_OF_PARAMETER_LOCKS; parameter_number++)
          {
            m.local_parameter_lock_settings.setParameter(parameter_number, getParameter(parameter_number, m.playback_position));
          }

          // If the sample start settings is set and snap is on, then quantize the sample start position.
          float sample_start = m.local_parameter_lock_settings.getParameter(SAMPLE_START);

          if (sample_position_snap_value > 0 && sample_start > 0)
          {
            // settings.sample_start ranges from 0 to 1
            // This next line sets quantized_sample_start to an integer between 0 and sample_position_snap
            // float quantized_sample_start = settings.sample_start * (float)sample_position_snap_value;
            float quantized_sample_start = sample_start * (float)sample_position_snap_value;
            quantized_sample_start = std::floor(quantized_sample_start);
            m.local_parameter_lock_settings.setParameter(SAMPLE_START, quantized_sample_start / (float)sample_position_snap_value);
          }

          // Trigger the ADSR
          adsr.gate(true);

          // trigger sample playback
          sample_player->trigger(sample_start, m.local_parameter_lock_settings.getParameter(REVERSE));

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

      if (m.steps[m.playback_position] && (m.skipped == false))
      {
        // unsigned int ratchet_pattern = settings.parameters[RATCHET] * (NUMBER_OF_RATCHET_PATTERNS - 1);
        unsigned int ratchet_pattern = m.local_parameter_lock_settings.getParameter(RATCHET) * (NUMBER_OF_RATCHET_PATTERNS - 1);
        if (ratchet_patterns[ratchet_pattern][m.ratchet_counter])
        {
          sample_player->trigger(m.local_parameter_lock_settings.getParameter(SAMPLE_START), m.local_parameter_lock_settings.getParameter(REVERSE));
          adsr.gate(true); // retrigger the ADSR
          ratcheted = true;
        }
        if (++m.ratchet_counter >= 8)
          m.ratchet_counter = 0;
      }
      else
      {
        m.ratchet_counter = 0;
      }

      return (ratcheted);
    }

    unsigned int getPosition()
    {
      return (m.playback_position);
    }

    void setPosition(unsigned int playback_position)
    {
      if (playback_position < NUMBER_OF_STEPS)
      {
        this->m.playback_position = playback_position;
      }
    }

    void toggleStep(unsigned int i)
    {
      m.steps[i] ^= true;
    }

    bool getValue(unsigned int i)
    {
      return (m.steps[i]);
    }

    void setValue(unsigned int i, bool value)
    {
      m.steps[i] = value;
    }

    void reset()
    {
      m.playback_position = m.range_start;
      m.ratchet_counter = 0;
      fade_out.reset();
    }

    void clear()
    {
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
      {
        setValue(i, false);
      }
      this->m.range_end = NUMBER_OF_STEPS - 1;
      this->m.range_start = 0;
      this->resetAllParameterLocks();
    }

    void clearSteps()
    {
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
      {
        setValue(i, false);
      }
    }

    void clearParameters()
    {
      this->resetAllParameterLocks();
    }

    void clearStepParameters(unsigned int step_id)
    {
      for (unsigned int parameter_number = 0; parameter_number < NUMBER_OF_PARAMETER_LOCKS; parameter_number++)
      {
        setParameter(parameter_number, step_id, default_parameter_values[parameter_number]);
      }
    }

    /* Possibly adding this in the future
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
        ParameterLockSettings temp_settings[NUMBER_OF_STEPS];
        bool temp_steps[NUMBER_OF_STEPS];

        for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
        {
          temp_settings[i].copy(&m.parameter_lock_settings[i]);
          temp_steps[i] = this->m.steps[i];
        }

        // Now copy the track information back into the shifted location
        for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
        {
          unsigned int copy_from_index = (i + amount) % NUMBER_OF_STEPS;
          m.parameter_lock_settings[i].copy(&temp_settings[copy_from_index]);
          this->m.steps[i] = temp_steps[copy_from_index];
        }
      }
    }

    void copyStep(unsigned int copy_from_index, unsigned int copy_to_index)
    {
      if (copy_from_index != copy_to_index)
      {
        m.parameter_lock_settings[copy_to_index].copy(&m.parameter_lock_settings[copy_from_index]);
        this->m.steps[copy_to_index] = this->m.steps[copy_from_index];
      }
    }

    void initialize()
    {
      this->clear();
      this->sample_player->initialize();
    }

    void getStereoOutput(float *final_left_output, float *final_right_output, unsigned int interpolation)
    {
      float left_output = 0.0;
      float right_output = 0.0;

      // -===== Slew Limiter Processing =====-
      float volume = volume_slew_limiter->process(APP->engine->getSampleTime(), m.local_parameter_lock_settings.getParameter(VOLUME));
      float filter_cutoff = filter_cutoff_slew_limiter->process(APP->engine->getSampleTime(), m.local_parameter_lock_settings.getParameter(FILTER_CUTOFF));
      float filter_resonance = filter_resonance_slew_limiter->process(APP->engine->getSampleTime(), m.local_parameter_lock_settings.getParameter(FILTER_RESONANCE));
      // Pan is processed below, after the track pan is applied

      // Mostly cosmetic: Load up the settings into easily readable variables
      // The "m.local_parameter_lock_settings" structure is populated when the track is stepped.  It
      // contains of snapshot of the m.local_parameter_lock_settings related to the active step.

      float pan = m.local_parameter_lock_settings.getParameter(PAN);
      float attack = m.local_parameter_lock_settings.getParameter(ATTACK);
      float release = m.local_parameter_lock_settings.getParameter(RELEASE);
      float delay_mix = m.local_parameter_lock_settings.getParameter(DELAY_MIX);
      float delay_length = m.local_parameter_lock_settings.getParameter(DELAY_LENGTH);
      float delay_feedback = m.local_parameter_lock_settings.getParameter(DELAY_FEEDBACK);

      // When the ADSR reaches the sustain state, then switch to the release
      // state.  Only do this when the release is less than max release, otherwise
      // sustain until the next time the track is triggered.
      //
      // * Reminder: m.local_parameter_lock_settings.getParameter(RELEASE) ranges from 0.0 to 1.0
      //
      if (adsr.getState() == ADSR::env_sustain && release < 1.0)
        adsr.gate(false);

      // Read sample output
      this->sample_player->getStereoOutput(&left_output, &right_output, interpolation);

  

      // Apply pan parameters
      //
      // m.local_parameter_lock_settings.pan ranges from 0 to 1
      // track_pan ranges from -1 to 0

      float computed_pan = (pan * 2.0) - 1.0; // convert m.local_parameter_lock_settings.pan to range from -1 to 1
      computed_pan = clamp(computed_pan + m.track_pan, -1.0, 1.0); // 6.5 to 7
      computed_pan = pan_slew_limiter->process(APP->engine->getSampleTime(), computed_pan);

      if(computed_pan != 0)
      {
        stereo_pan.process(&left_output, &right_output, computed_pan);
      }

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

      // -===== ADSR Processing =====-

      if(attack > 0.0 || release < 1.0) // skip computations if not used
      {
        adsr.setAttackRate(attack * APP->engine->getSampleRate());
        adsr.setReleaseRate(release * maximum_release_time * APP->engine->getSampleRate());
        float adsr_value = adsr.process();

        // Apply ADSR to volume
        left_output *= adsr_value;
        right_output *= adsr_value;
      }

      // Apply filter
      if(filter_cutoff < 1.0) // Skip if cutoff is max value
      {
        filter.setCutoff(filter_cutoff);
        filter.setResonance(filter_resonance);
        filter.process(&left_output, &right_output);
      }

      // If the delay mix is above 0 for this track, then compute the delay and
      // output it.  This if statement is an attempt to trim down CPU usage when
      // the delay is effectively turned off.
      if (delay_mix > 0)
      {
        float delay_output_left = 0.0;
        float delay_output_right = 0.0;

        // Apply delay
        delay->setMix(delay_mix);
        delay->setBufferSize(delay_length * (APP->engine->getSampleRate() / 4));
        delay->setFeedback(delay_feedback);
        delay->process(left_output, right_output, delay_output_left, delay_output_right);

        // Return audio with the delay applied
        *final_left_output = delay_output_left;
        *final_right_output = delay_output_right;
      }
      else
      {
        *final_left_output = left_output;
        *final_right_output = right_output;        
      }
    }

    void incrementSamplePosition()
    {
      float pitch = m.local_parameter_lock_settings.getParameter(PITCH);
      float reverse = m.local_parameter_lock_settings.getParameter(REVERSE);
      float sample_start = m.local_parameter_lock_settings.getParameter(SAMPLE_START);
      float sample_end = m.local_parameter_lock_settings.getParameter(SAMPLE_END);
      float loop = m.local_parameter_lock_settings.getParameter(LOOP);

      float summed_pitch = clamp(pitch + m.track_pitch, 0.0, 1.0);
      float rescaled_pitch = rescale(summed_pitch, 0.0, 1.0, -2.0, 2.0);

      if (reverse > .5)
      {
        // -2.0 to 2.0 is a two octave range in either direction (4 octives total)
        this->sample_player->stepReverse(rescaled_pitch, sample_start, sample_end, loop);
      }
      else
      {
        this->sample_player->step(rescaled_pitch, sample_start, sample_end, loop);
      }
    }

    void updateRackSampleRate()
    {
      this->sample_player->updateSampleRate();
    }

    bool isFadingOut()
    {
      return (fade_out.fading_out);
    }

    void copy(Track *src_track)
    {
      this->m = src_track->m;
    }

    void randomizeSteps()
    {
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
      {
        m.steps[i] = (rand() > (RAND_MAX / 2));
      }
    }

    unsigned int getRangeStart()
    {
      return (this->m.range_start);
    }

    void setRangeStart(unsigned int range_start)
    {
      if (m.playback_position < range_start) m.playback_position = range_start;
      this->m.range_start = range_start;
    }

    unsigned int getRangeEnd()
    {
      return (this->m.range_end);
    }

    void setRangeEnd(unsigned int range_end)
    {
      this->m.range_end = range_end;
    }

    // Parameter locks
    // ============================================================================
    void resetAllParameterLocks()
    {
      for (unsigned int step = 0; step < NUMBER_OF_STEPS; step++)
      {
        for (unsigned int parameter_number = 0; parameter_number < NUMBER_OF_PARAMETER_LOCKS; parameter_number++)
        {
          setParameter(parameter_number, step, default_parameter_values[parameter_number]);
        }
      }
    }

    // Be careful here.  setParameter and getParameter are helper functions.  There's 
    // also similar methods in ParameterLockSettings.hpp which are specific
    // to a track's current step's parameters.

    float getParameter(unsigned int parameter_number, unsigned int step)
    {
      return (m.parameter_lock_settings[step].getParameter(parameter_number)); // [parameter_number])
    }

    void setParameter(unsigned int parameter_number, unsigned int step, float value)
    {
      m.parameter_lock_settings[step].setParameter(parameter_number, value);
    }

    // "track_pan" is the global pan applied by the expander module
    float getTrackPan()  {
      return(this->m.track_pan);
    }
    void setTrackPan(float track_pan) {
      this->m.track_pan = track_pan;
    }

    // "track_pitch" is the global pitch applied by the expander module
    float getTrackPitch()  {
      return(this->m.track_pitch);
    }
    void setTrackPitch(float track_pitch) {
      this->m.track_pitch = track_pitch;
    }

  };

}
