struct Grain
{
  // Start Position is the offset into the sample where playback should start.
  // double start_position;

  float start_position = 0.0;

  // Playback length for the grain, measuring in .. er.. ticks?
  // double playback_length;

  // sample_ptr points to the loaded sample in memory
  Sample *sample_ptr;

  // Eventually use inheritance to purge this sloppy pointer passing
  // Common *common;

  // playback_position is similar to samplePos used in for samples.  However,
  // it's relative to the Grain's start_position rather than the sample
  // start position.
  double playback_position = 0.0f;
  float pan = 0;
  int sample_position = 0;
  unsigned int age = 0;
  unsigned int lifespan = 0;
  float step_amount = 0;

  float output_voltage_left = 0;
  float output_voltage_right = 0;
  bool erase_me = false;

  StereoPan stereo_pan; // vgLib-2.0/dsp/StereoPan.hpp

  Grain()
  {
  }

  std::pair<float, float> getStereoOutput()
  {
    if(age == 0) return {0,0};

    // Note that we're adding two floating point numbers, then casting them to an int, which is much faster than using floor()
    sample_position = (this->start_position + this->playback_position);
    sample_position %= this->sample_ptr->size(); // be careful about division by 0 here

    this->sample_ptr->read(sample_position, &output_voltage_left, &output_voltage_right);


    // Apply amplitude slope

    int slope_index = (1.0 - ((float)age / (float)lifespan)) * 512.0;  // remember that age decrements instead of increments
    slope_index = clamp(slope_index, 0, 511);

    // At the moment, this slope_index is always 1, so the contour selection doesn't make a difference.  I had
    // at one time thought that offering differeing amplidue contours would be fun, but it ended up making
    // so little difference that I removed this feature.
    float slope_value = CONTOUR[slope_index];

    output_voltage_left  = slope_value * output_voltage_left;
    output_voltage_right = slope_value * output_voltage_right;

    // Apply pan
    stereo_pan.process(&output_voltage_left, &output_voltage_right, pan);

    return {output_voltage_left, output_voltage_right};
  }

  void step()
  {
    if(erase_me == false)
    {
      playback_position = playback_position + step_amount;
      // void step(float pitch = 0.0, float sample_start = 0.0, float sample_end = 1.0, bool loop = false)
      // sample_player_ptr->step(pitch, start_position, 1.0);
      if(! --age) erase_me = true;
    }
  }

  unsigned int getSamplePosition()
  {
    return(this->start_position + this->playback_position);
  }
};
