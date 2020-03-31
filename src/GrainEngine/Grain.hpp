struct Grain
{
    // Start Position is the offset into the sample where playback should start.
    // It is set when the ghost is first created.
    float start_position;

    // Playback length for the ghost, measuring in .. er.. ticks?
    float playback_length;

    // sample_ptr points to the loaded sample in memory
    Sample *sample_ptr;

    // playback_position is similar to samplePos used in for samples.  However,
    // it's relative to the Ghost's start_position rather than the sample
    // start position.
    float playback_position = 0.0f;
    float pan = 0;
    unsigned int sample_position = 0;

    float output_voltage_left = 0;
    float output_voltage_right = 0;

    bool erase_me = false;

    // StereoFadeOutSubModule stereo_fade_out;
    // StereoFadeInSubModule stereo_fade_in;
    StereoPanSubModule panner;

    Grain()
    {
        // Since the fade out isn't processed until the sample playback is past
        // a certain point, we can just trigger it now and it'll start working
        // later.
        // stereo_fade_out.trigger();
        // stereo_fade_in.trigger();
    }

    std::pair<float, float> getStereoOutput(float smooth_rate, int selected_slope)
    {
        if(playback_length == 0) return {0,0};

        // Note that we're adding two floating point numbers, then casting
        // them to an int, which is much faster than using floor()
        sample_position = this->start_position + this->playback_position;

        if(sample_position >= this->sample_ptr->total_sample_count)
        {
            // NOTE: Ideally, the sample position should be reaching the total sample
            // count (or length) exactly as an applied amp envelope is reaching 0
            erase_me = true;
        }
        else
        {
            output_voltage_left  = this->sample_ptr->leftPlayBuffer[sample_position];
            output_voltage_right = this->sample_ptr->rightPlayBuffer[sample_position];

            // Apply amplitude slope
            int slope_index = (playback_position / playback_length) * 512.0;
            slope_index = clamp(slope_index, 0, 511);
            selected_slope = clamp(selected_slope, 0, 9);

            float slope_value = GRAIN_AMP_SLOPES[selected_slope][slope_index];

            output_voltage_left  = (slope_value / 256.0) * output_voltage_left;
            output_voltage_right = (slope_value / 256.0) * output_voltage_right;

            // Apply pan
            std::tie(output_voltage_left, output_voltage_right) = panner.process(output_voltage_left, output_voltage_right, pan);
        }

        return {output_voltage_left, output_voltage_right};
    }

    void step(float step_amount)
    {
        if(erase_me == false)
        {
            // Step the playback position forward.
            playback_position = playback_position + step_amount;
            if(playback_position >= playback_length) erase_me = true;
        }
    }

};
