struct GrainBlenderEx
{
    std::deque<Grain> grain_queue;

    GrainBlenderEx()
    {
    }

    virtual ~GrainBlenderEx() {
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

    virtual void add(float start_position, unsigned int lifespan, float pan, AudioBuffer *buffer_ptr, unsigned int max_grains)
    {
        if(grain_queue.size() > max_grains) return;
        if(lifespan == 0) return;

        Grain grain;

        // Configure it for playback
        grain.start_position = start_position;
        grain.buffer_ptr = buffer_ptr;
        grain.lifespan = lifespan;
        grain.age = lifespan;
        grain.pan = pan;

        grain_queue.push_back(grain);
    }

    virtual std::pair<float, float> process(float smooth_rate, float step_amount)
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
                std::pair<float, float> stereo_output = grain.getStereoOutput(smooth_rate);
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
