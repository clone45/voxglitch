struct FourTrack : VoxglitchSamplerModule
{

    TrackModel track;
    Sample sample;
    WaveformModel waveform_model;

    dsp::SchmittTrigger start_trigger;
    dsp::SchmittTrigger stop_trigger;
    dsp::SchmittTrigger reset_trigger;

    // Add these new members
    bool playing = false;
    unsigned int playback_position = 0;
    float output_left = 0.0f;
    float output_right = 0.0f;
    int active_marker = 0;  // Track which marker is currently active

    enum ParamIds
    {
        ENUMS(MARKER_BUTTONS, 32),
        NUM_PARAMS
    };
    enum InputIds
    {
        START_INPUT,
        STOP_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT_LEFT,
        OUTPUT_RIGHT,
        ENUMS(MARKER_OUTPUTS, 32),
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(MARKER_LIGHTS, 32),
        NUM_LIGHTS
    };

    FourTrack()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        sample.load("e:/dev/example.wav");
        track.setSample(&sample);

        // Set the waveform model sample for the lower waveform display
        waveform_model.sample = &sample;
        waveform_model.visible = true;

        // Configure parameters and their corresponding lights
        for (int i = 0; i < 32; i++) {
            configParam(MARKER_BUTTONS + i, 0.f, 1.f, 0.f, "Marker Selection Button #" + std::to_string(i));
            configLight(MARKER_LIGHTS + i, "Select #" + std::to_string(i));
        }

        for (int i = 0; i < 32; i++) {
            configParam(MARKER_BUTTONS + i, 0.f, 1.f, 0.f, "Marker Selection Button #" + std::to_string(i));
        }
        params[MARKER_BUTTONS].setValue(1.f);
        active_marker = 0;
    }

    // Autosave module data.  VCV Rack decides when this should be called.
    json_t *dataToJson() override
    {
        json_t *root = json_object();
        return root;
    }

    // Load module data
    void dataFromJson(json_t *root) override
    {
    }

    void process(const ProcessArgs &args) override
    {
        bool reset_triggered = reset_trigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);
        bool start_triggered = start_trigger.process(inputs[START_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);
        bool stop_triggered = stop_trigger.process(inputs[STOP_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);

        if (start_triggered)
        {
            playing = true;
        }

        if (stop_triggered)
        {
            playing = false;
        }

        if (reset_triggered)
        {
            playback_position = 0;
        }

        // Handle playback
        if (playing && sample.loaded)
        {
            if (playback_position < sample.size())
            {
                sample.read(playback_position, &output_left, &output_right);
                playback_position++;

                // Update waveform position indicator
                waveform_model.playback_percentage = float(playback_position) / float(sample.size());
            }
            else
            {
                // Reached end of sample
                playing = false;
                playback_position = 0;
                waveform_model.playback_percentage = 0.0f;
            }
        }

        outputs[OUTPUT_LEFT].setVoltage(output_left * 5.0f);  // Scale by 5V for typical modular levels
        outputs[OUTPUT_RIGHT].setVoltage(output_right * 5.0f);

        // Handle marker button changes
        for (int i = 0; i < 32; i++) {
            // If this button was just pressed
            if (params[MARKER_BUTTONS + i].getValue() > 0.f && i != active_marker) {
                // Clear old active button
                params[MARKER_BUTTONS + active_marker].setValue(0.f);
                // Set new active button
                active_marker = i;
                params[MARKER_BUTTONS + i].setValue(1.f);
                // Here you can add any other logic needed when a marker changes
            }
            // Ensure active marker stays pressed
            else if (i == active_marker && params[MARKER_BUTTONS + i].getValue() == 0.f) {
                params[MARKER_BUTTONS + i].setValue(1.f);
            }
            // Update lights to match button states
            lights[MARKER_LIGHTS + i].setBrightness(params[MARKER_BUTTONS + i].getValue());
        }      
    }
};
