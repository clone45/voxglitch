struct GrainManager
{
    Grain grain_array[MAX_GRAINS];
    Grain grain_array_tmp[MAX_GRAINS];
    unsigned int grain_array_length = 0;
    // Common *common;

    GrainManager()
    {
    }

    virtual ~GrainManager() {
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

    void purge()
    {
      grain_array_length = 0;
    }

    virtual void addGrain(double start_position, unsigned int lifespan, float pan, Sample *sample_ptr, unsigned int max_grains, float step_amount)
    {
        if(grain_array_length > max_grains || (grain_array_length >= (MAX_GRAINS - 1))) return;
        if(lifespan == 0) return;

        Grain grain;

        // Configure grain for playback
        grain.start_position = start_position;
        grain.sample_ptr = sample_ptr;
        grain.lifespan = lifespan;
        grain.age = lifespan;
        grain.pan = pan;
        grain.step_amount = step_amount;

        grain_array[grain_array_length] = grain;

        grain_array_length ++;
    }

    virtual std::pair<float, float> process()
    {
        float left_mix_output = 0;
        float right_mix_output = 0;
        unsigned int grain_array_tmp_length = 0;

        //
        // Process grains
        // ---------------------------------------------------------------------

        for (unsigned int i=0; i < grain_array_length; i++)
        {
            if(grain_array[i].erase_me == false)
            {
                std::pair<float, float> stereo_output = grain_array[i].getStereoOutput();
                left_mix_output  += stereo_output.first;
                right_mix_output += stereo_output.second;

                grain_array[i].step();

                grain_array_tmp[grain_array_tmp_length] = grain_array[i];
                grain_array_tmp_length++;
            }
        }

        std::swap(grain_array, grain_array_tmp);
        grain_array_length = grain_array_tmp_length;


        return {left_mix_output, right_mix_output};
    }

};
