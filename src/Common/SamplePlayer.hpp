/*
common version of SamplePlayer

Let's plan out the new un-clicking strategy...

When it's time to crossface, I need to keep track of:
1. The new position of the read head
2. The old position of the read head

While crossfading, I need to increment both
Once crossfading is done, I can stop incrementing the old position of the read
head.

- I have to keep track of if the sample player is in the process of crossfading
- I have to keep track of the crossfading progress (as a float percentage?)



*/

struct SamplePlayer
{
  Sample sample;
  Xfade xfade;
  // double playback_position = 0.0f;

  // These will take the place of playback_position
  double old_playhead = 0.0f;
  double playhead = 0.0f;

  bool playing = false;
  double step_amount = 0.0;
  unsigned int debug_count = 150;

  /*
  void getStereoOutput(float *left_output, float *right_output, unsigned int interpolation)
  {
    if(crossfading)
    {
      getCrossfadingStereoOutput(left_output, right_output, interpolation);
    }
    else
    {
      unsigned int sample_index = playback_position; // convert float to int

      if((playing == false) || ((unsigned int) sample_index >= this->sample.size()) || (sample.loaded == false))
      {
        *left_output = 0;
        *right_output = 0;
      }
      else
      {
        if(interpolation == 0)
        {
          // Normal version, using sample index
          this->sample.read(sample_index, left_output, right_output);
        }
        else
        {
          // Read sample using Linear Interpolation, sending in double
          this->sample.readLI(playback_position, left_output, right_output);
        }
      }
    }
  }
  */

  void getStereoOutput(float *left_output, float *right_output, unsigned int interpolation)
  {
    if(xfade.crossfading)
    {
      getCrossfadingStereoOutput(left_output, right_output, interpolation);
    }
    else
    {
      unsigned int sample_index = playhead; // convert float to int

      if((playing == false) || ((unsigned int) sample_index >= this->sample.size()) || (sample.loaded == false))
      {
        *left_output = 0;
        *right_output = 0;
      }
      else
      {
        if(interpolation == 0)
        {
          // Normal version, using sample index
          this->sample.read(sample_index, left_output, right_output);
        }
        else
        {
          // Read sample using Linear Interpolation, sending in double
          this->sample.readLI(playhead, left_output, right_output);
        }
      }
    }
  }

  void getCrossfadingStereoOutput(float *left_output, float *right_output, unsigned int interpolation)
  {
    float playhead_left_output;
    float playhead_right_output;
    float old_playhead_left_output;
    float old_playhead_right_output;

    unsigned int sample_index = playhead; // convert float to int
    unsigned int old_sample_index = old_playhead;

    if((playing == false) || (sample_index >= this->sample.size()) || (sample.loaded == false))
    {
      *left_output = 0;
      *right_output = 0;
    }
    else
    {
      if(interpolation == 0)
      {
        this->sample.read(sample_index, &playhead_left_output, &playhead_right_output);
        this->sample.read(old_sample_index, &old_playhead_left_output, &old_playhead_right_output);
      }
      else
      {
        this->sample.readLI(playhead, &playhead_left_output, &playhead_right_output);
        this->sample.readLI(old_playhead, &old_playhead_left_output, &old_playhead_right_output);
      }

      // *left_output = crossfade(crossfade_percent, old_playhead_left_output, playhead_left_output);
      // *right_output = crossfade(crossfade_percent, old_playhead_right_output, playhead_right_output);

      *left_output = xfade.process(old_playhead_left_output, playhead_left_output);
      *right_output = xfade.process(old_playhead_right_output, playhead_right_output);



      /*
      if(debug_count != 0)
      {
        DEBUG("crossfade_percent");
        DEBUG(std::to_string(crossfade_percent).c_str());
        debug_count--;
      }
      */
    }
  }

  /*
  void trigger(float sample_start = 0, bool reverse = false)
  {
    if(! reverse) // if forward playback
    {
      this->playback_position = sample_start * this->sample.size();
    }
    else // if reverse playback
    {
      this->playback_position = (( 1 - sample_start) * this->sample.size());
    }

    this->playing = true;
  }
  */

  void trigger(float sample_start = 0, bool reverse = false)
  {
    // Start crossfading if the sample start is not 0, or if the sample is
    // being retriggered.  Don't crossfade if the sample is triggered from
    // the beginning.
    if(sample_start != 0 && this->playing == true)
    {
      // Position the old_playhead to the location in the sample before we
      // move the playhead.
      this->old_playhead = this->playhead;

      // begin crossfading
      xfade.trigger();
    }

    if(! reverse) // if forward playback
    {
      this->playhead = sample_start * this->sample.size();
    }
    else // if reverse playback
    {
      this->playhead = (( 1 - sample_start) * this->sample.size());
    }

    this->playing = true;
  }

  // Parameters:
  //   pitch - range should be bipolar, ranging from ±5.0
  //   sample_start - unipolor, from 0.0 to 1.0
  //   sample_end - unipolor, from 0.0 to 1.0
  //   loop - boolean (true or false)

  void step(float pitch = 0.0, float sample_start = 0.0, float sample_end = 1.0, bool loop = false)
  {
    if(this->playing && this->sample.loaded)
    {
      double sample_increment = getSampleIncrement(pitch);

      playhead += sample_increment;

      // If crossfading, step the old playhead and increment the crossfade amount.
      if (xfade.crossfading)
      {
        old_playhead += sample_increment;
        xfade.step();
      }

      // If settings loop is greater than 0 and the sample position is past the
      // selected loop length, then loop.  Note:  If loop is set to 1, then
      // the entire sample will loop.

      unsigned int sample_size = sample.size() * sample_end;

      if(loop > 0)
      {
        float loop_position = (sample_start * sample_size) + ((sample_size - sample_start) * loop);

        // Check to see if playhead is past the loop point
        if(playhead >= loop_position) playhead = (sample_start * sample_size);
        if(xfade.crossfading && old_playhead >= loop_position) old_playhead = (sample_start * sample_size);
      }
      else
      {
        if(playhead >= sample_size) stop();
      }
    }
  }

  void stepReverse(float pitch = 0.0, float sample_start = 0.0, float sample_end = 1.0, bool loop = false)
  {
    if(this->playing && this->sample.loaded)
    {
      double sample_increment = getSampleIncrement(pitch);

      // Step the playback position backward.
      playhead -= sample_increment;

      if (xfade.crossfading)
      {
        old_playhead -= sample_increment;
        xfade.step();
      }

      unsigned int sample_size = sample.size() * sample_end;

      // If the playback position is past the beginning, end or loop sample playback
      if(loop > 0)
      {
        float playback_start = (1.0 - sample_start) * sample_size;
        float loop_position = playback_start - (loop * (sample_size - playback_start));

        if(playhead <= loop_position) playhead = playback_start;
        if(xfade.crossfading && old_playhead <= loop_position) old_playhead = playback_start;
      }
      else if(playhead <= 0) stop();
    }
  }

  double getSampleIncrement(float pitch_cv_input)
  {
    return(this->step_amount * exp2(pitch_cv_input));
  }

  void stop()
  {
    playing = false;
  }

  void loadSample(std::string path)
  {
    sample.load(path);
    updateStepAmount();
  }

  std::string getFilename()
  {
    return(sample.filename);
  }

  void setFilename(std::string filename)
  {
    sample.filename = filename;
  }

  std::string getPath()
  {
    return(sample.path);
  }

  void setPath(std::string path)
  {
    sample.path = path;
  }

  void updateSampleRate()
  {
    updateStepAmount();
  }

  void updateStepAmount()
  {
    step_amount = (sample.sample_rate / APP->engine->getSampleRate());
  }

  void initialize()
  {
    sample.unload();
    this->playhead = 0.0f;
    this->playing = false;
    this->setFilename("");
    this->setPath("");
  }
};
