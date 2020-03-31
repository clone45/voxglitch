struct GrainEngineEx
{
    std::deque<Grain> grain_queue;

    GrainEngineEx()
    {
    }

    virtual ~GrainEngineEx() {
    }

    // Return number of active grains
    virtual int size()
    {
        return(grain_queue.size());
    }

    virtual bool isEmpty()
    {
        return(grain_queue.empty());
    }

    virtual void add(float start_position, float playback_length, float pan, Sample *sample_ptr)
    {
        if(grain_queue.size() > MAX_GRAINS) return;
        if(playback_length == 0) return;

        Grain grain;

        // Configure it for playback
        grain.start_position = start_position;
        grain.playback_length = playback_length;
        grain.sample_ptr = sample_ptr;
        grain.pan = pan;

        grain_queue.push_back(grain);
    }

    virtual std::pair<float, float> process(float smooth_rate, float step_amount, int selected_slope)
    {
        float left_mix_output = 0;
        float right_mix_output = 0;

        //
        // Process grains
        // ---------------------------------------------------------------------

        for (Grain &grain : grain_queue)
        {
            if(grain.erase_me != true)
            {
                std::pair<float, float> stereo_output = grain.getStereoOutput(smooth_rate, selected_slope);
                left_mix_output  += stereo_output.first;
                right_mix_output += stereo_output.second;
                grain.step(step_amount);
            }
        }

        // perform cleanup of grains ready for removal
        grain_queue.erase(std::remove_if(
            grain_queue.begin(), grain_queue.end(),
                [](const Grain& grain) {
                    return grain.erase_me;
                }), grain_queue.end());

        return {left_mix_output, right_mix_output};
    }

};
