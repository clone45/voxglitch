struct DrumRandomizer : VoxglitchModule
{
    dsp::SchmittTrigger step_trigger;
    dsp::SchmittTrigger reset_trigger;

    unsigned int sequence_length = 16;
    float sequences[16][16]; // 16 steps, 16 channels
    bool passthrough[16];

    unsigned int step = 0;

    unsigned int old_channel_knob_selection = 0;    
    unsigned int old_step_knob_selection = 0;

    dsp::TTimer<double> reset_timer;
    bool first_step = true;
    bool wait_for_reset_timer = false;

    // These are variable that are used by the readout widgets in order to 
    // "watch" as their data sources.
    float channel_display_value = 0.0;
    float step_display_value = 0.0;
    float percentage_display_value = 0.0;

    Random random;

    enum ParamIds
    {
        CHANNEL_KNOB,
        STEP_KNOB,
        PERCENTAGE_KNOB,
        NUM_PARAMS
    };
    enum InputIds
    {
        GATE_INPUT,
        STEP_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        GATE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    void clearAllSequencers()
    {
        for(unsigned int channel=0; channel<16; channel++)
        {
            clearSequence(channel);
        }
    }

    void clearSequence(unsigned int channel)
    {
        for(unsigned int i=0; i<16; i++)
        {
            setSequenceValue(channel, i, 1.0); // 1.0 == 100% playback
        }
    }

    void setSequenceValue(unsigned int channel, unsigned int step, bool value)
    {
        sequences[channel][step] = value;
    }

    // Save module data
    json_t *dataToJson() override
    {
        json_t *json_root = json_object();

        json_t *sequences_json_array = json_array();

        for (unsigned int channel = 0; channel < 16; channel++) 
        {
            json_t *channel_json_array = json_array();

            for (unsigned int step = 0; step < 16; step++) 
            {
                json_array_append_new(channel_json_array, json_real(this->sequences[channel][step]));
            }
            json_array_append_new(sequences_json_array, channel_json_array);
        }

        json_object_set(json_root, "sequences", sequences_json_array);
        json_decref(sequences_json_array);

        return json_root;
    }

    // Load module data
    void dataFromJson(json_t *root) override
    {
        json_t *sequences_json_array = json_object_get(root, "sequences");

        for (unsigned int channel = 0; channel < 16; channel++)
        {
            json_t *channel_json_array = json_array_get(sequences_json_array, channel);

            for (unsigned int step = 0; step < 16; step++)
            {
                this->sequences[channel][step] = json_real_value(json_array_get(channel_json_array, step));
            }
        }
    }        

    void reset()
    {
        first_step = true;
        step = 0;
        wait_for_reset_timer = true;

        // Set up a counter so that the clock input will ignore
        // incoming clock pulses for 1 millisecond after a reset input. This
        // is to comply with VCV Rack's standards.  See section "Timing" at
        // https://vcvrack.com/manual/VoltageStandards

        reset_timer.reset();
    }

    bool process_step_input()
    {
        // Process STEP input
        if (!wait_for_reset_timer && step_trigger.process(inputs[STEP_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
        {
            // If there's a step input, but first_step is true, then don't
            // increment the step and output the value at step #1
            if (first_step)
            {
                first_step = false;
            }
            // Otherwise, step the sequencer
            else
            {
                step++;

                // If we're at the end of the sequencer, wrap to the beginning
                if (step == sequence_length)
                {
                    step = 0;
                }
            }
            
            return(true);
        }

        if (wait_for_reset_timer)
        {
            // Accumulate time in reset_timer
            if (reset_timer.process(APP->engine->getSampleTime() * 1000.0) > 1.0)
            {
                wait_for_reset_timer = false;
                reset_timer.reset();
            }
        }

        return(false);
    } 

    void initilizePassthrough()
    {
        for(unsigned int channel=0; channel<16; channel++)
        {
            passthrough[channel] = true;
        }        
    }

    DrumRandomizer()
    {
        clearAllSequencers();
        initilizePassthrough();

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(CHANNEL_KNOB, 0.0f, 15, 0.0f, "ChannelKnob");
        paramQuantities[CHANNEL_KNOB]->snapEnabled = true;
        
        configParam(STEP_KNOB, 0.0f, 15, 0.0f, "Step Knob");
        paramQuantities[STEP_KNOB]->snapEnabled = true;

        configParam(PERCENTAGE_KNOB, 0.0f, 1.0f, 1.0f, "Percentage Knob");
    }

    void process(const ProcessArgs &args) override
    {
        // 
        // Process knobs
        //

        unsigned int channel_knob_selection = params[CHANNEL_KNOB].getValue();
        unsigned int step_knob_selection = params[STEP_KNOB].getValue();
        
        // Communicate choice to the channel readout widget
        channel_display_value = channel_knob_selection;
        step_display_value = step_knob_selection + 1;
        // step_display_value = (int) step;

        // If channel knob changes, then update percentage knob
        if(channel_knob_selection != old_channel_knob_selection)
        {
            old_channel_knob_selection = channel_knob_selection;
            params[PERCENTAGE_KNOB].setValue(sequences[channel_knob_selection][step_knob_selection]);
        }

        // If step knob changes, then update percentage knob
        if(step_knob_selection != old_step_knob_selection)
        {
            old_step_knob_selection = step_knob_selection;
            params[PERCENTAGE_KNOB].setValue(sequences[channel_knob_selection][step_knob_selection]);
        }

        // Finally, set the percentage value.  Note, this really only makes
        // a difference when the user turns the percentage knob.  This stores
        // the selected percentage into our sequences array.
        float percentage_knob_selection = params[PERCENTAGE_KNOB].getValue();
        sequences[channel_knob_selection][step_knob_selection] = percentage_knob_selection;
        percentage_display_value = percentage_knob_selection;

        //
        // Process Reset
        //
        if(reset_trigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
        {
            reset();
        }

        //
        // Process stepping
        //

        bool stepped = process_step_input();

        if(stepped)
        {
            for(unsigned int channel = 0; channel < 16; channel++)
            {
                float r = random.gen();

                // Here, "step" refers to the step that the sequencer is on,
                // not the value of the step knob, which is used for editing.
                if(r < sequences[channel][step])
                {
                    passthrough[channel] = true;
                }
                else
                {
                    passthrough[channel] = false;
                }
            }
        }

        outputs[GATE_OUTPUT].setChannels(16);

        for(unsigned int channel = 0; channel < 16; channel++)
        {
            if(passthrough[channel])
            {
                outputs[GATE_OUTPUT].setVoltage(inputs[GATE_INPUT].getVoltage(channel), channel);
            }
        }
    }
};
