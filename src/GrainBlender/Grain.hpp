struct Grain
{
    // Start Position is the offset into the sample where playback should start.
    float start_position;

    // Playback length for the grain, measuring in .. er.. ticks?
    float playback_length;

    // sample_ptr points to the loaded sample in memory
    AudioBuffer *buffer_ptr;

    // playback_position is similar to samplePos used in for samples.  However,
    // it's relative to the Grain's start_position rather than the sample
    // start position.
    float playback_position = 0.0f;
    float pan = 0;
    unsigned int sample_position = 0;
    unsigned int age = 0;
    unsigned int lifespan = 0;
    float pitch = 0;

    float output_voltage_left = 0;
    float output_voltage_right = 0;
    bool erase_me = false;

    StereoPanSubModule panner;

    Grain()
    {
    }

    std::pair<float, float> getStereoOutput(unsigned int contour_selection)
    {
        if(age == 0) return {0,0};

        // Note that we're adding two floating point numbers, then casting
        // them to an int, which is much faster than using floor()
        sample_position = this->start_position + this->playback_position;

        if(sample_position >= this->buffer_ptr->getBufferSize())
        {
            // NOTE: Ideally, the sample position should be reaching the total sample
            // count (or length) exactly as an applied amp envelope is reaching 0
            erase_me = true;
        }
        else
        {
            // output_voltage_left  = this->buffer_ptr->leftPlayBuffer[sample_position];
            // output_voltage_right = this->buffer_ptr->rightPlayBuffer[sample_position];
            /*
            this->buffer_ptr->step(sample_position);
            output_voltage_left  = this->buffer_ptr->getLeftValue();
            output_voltage_right = this->buffer_ptr->getRightValue();
            */
            std::tie(output_voltage_left, output_voltage_right) = this->buffer_ptr->getStereoOutput(sample_position);

            // Apply amplitude slope
            int slope_index = (1.0 - ((float)age / (float)lifespan)) * 512.0;  // remember that age decrements instead of increments
            slope_index = clamp(slope_index, 0, 511);
            float slope_value = CONTOURS[contour_selection][slope_index];

            output_voltage_left  = slope_value * output_voltage_left;
            output_voltage_right = slope_value * output_voltage_right;

            // Apply pan
            std::tie(output_voltage_left, output_voltage_right) = panner.process(output_voltage_left, output_voltage_right, pan);
        }

        return {output_voltage_left, output_voltage_right};
    }

    void step()
    {
        if(erase_me == false)
        {
            // Step the playback position forward.
            playback_position = playback_position + pitch;
            if(! --age) erase_me = true;
        }
    }

    unsigned int getSamplePosition()
    {
      return(this->start_position + this->playback_position);
    }
};
