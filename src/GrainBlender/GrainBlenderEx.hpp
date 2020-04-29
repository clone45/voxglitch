struct GrainBlenderEx
{
    // std::deque<Grain> grain_queue;
    Grain grain_array[MAX_GRAINS + 1];
    Grain grain_array_tmp[MAX_GRAINS + 1];
    unsigned int grain_array_length = 0;

    GrainBlenderEx()
    {
    }

    virtual ~GrainBlenderEx() {
    }

    // Return number of active grains
    virtual int size()
    {
        return(grain_array_length);
    }

    virtual bool isEmpty()
    {
        return(grain_array_length == 0);
    }

    virtual void add(float start_position, unsigned int lifespan, float pan, AudioBuffer *buffer_ptr, unsigned int max_grains, float pitch)
    {
        if(grain_array_length > max_grains) return;
        if(lifespan == 0) return;

        Grain grain;

        // Configure it for playback
        grain.start_position = start_position;
        grain.buffer_ptr = buffer_ptr;
        grain.lifespan = lifespan;
        grain.age = lifespan;
        grain.pan = pan;
        grain.pitch = pitch;

        grain_array[grain_array_length] = grain;
        grain_array_length ++;
    }

    virtual std::pair<float, float> process(float smooth_rate, unsigned int contour_selection)
    {
        float left_mix_output = 0;
        float right_mix_output = 0;
        unsigned int grain_array_tmp_length = 0;

        // unsigned int sample_position = 0;

        // TODO: Try this.  Instead of a second loop to erase grains, how about
        // maintaining two deque queues.  When a grain should not be removed,
        // it's pushed on to the second queue.  Then the queues are swapped?

        // And how about saying "fuck this" and not use a std::deque and instead
        // use an array with the maximum number set as MAX_GRAINS?

        //
        // Process grains
        // ---------------------------------------------------------------------

        for (unsigned int i=0; i < grain_array_length; i++)
        {
            if(grain_array[i].erase_me == false)
            {
                std::pair<float, float> stereo_output = grain_array[i].getStereoOutput(contour_selection);
                left_mix_output  += stereo_output.first;
                right_mix_output += stereo_output.second;

                grain_array[i].step();

                grain_array_tmp[grain_array_tmp_length] = grain_array[i];
                grain_array_tmp_length++;
            }
        }

        std::swap(grain_array, grain_array_tmp);
        grain_array_length = grain_array_tmp_length;

        // overlap_police.reset();

        // perform cleanup of grains ready for removal
        /*
        grain_queue.erase(std::remove_if(
            grain_queue.begin(), grain_queue.end(),
                [](const Grain& grain) {
                    return grain.erase_me;
                }), grain_queue.end());
        */

        return {left_mix_output, right_mix_output};
    }

};
