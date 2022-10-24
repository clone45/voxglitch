//
// TODO: Adjust pitch sensitivity and test
//

struct WavBank : VoxglitchSamplerModule
{
    unsigned int selected_sample_slot = 0;
    // double samplePos = 0;

    float SAMPLE_START_POSITION = 0.0;
    float SAMPLE_END_POSITION = 1.0;

    unsigned int trig_input_response_mode = TRIGGER;
    std::string rootDir;
    std::string path;
    bool loading_samples = false;

    std::vector<SamplePlayer> sample_players;
    dsp::SchmittTrigger playTrigger;
    DeclickFilter declick_filter;

    bool playback = false;

    enum ParamIds
    {
        WAV_KNOB,
        WAV_ATTN_KNOB,
        LOOP_SWITCH,
        NUM_PARAMS
    };
    enum InputIds
    {
        TRIG_INPUT,
        WAV_INPUT,
        PITCH_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        WAV_LEFT_OUTPUT,
        WAV_RIGHT_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    //
    // Constructor
    //
    WavBank()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(WAV_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
        configParam(WAV_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
        configParam(LOOP_SWITCH, 0.0f, 1.0f, 0.0f, "LoopSwitch");
        configInput(TRIG_INPUT, "Trigger");
        configInput(WAV_INPUT, "Wave Selection");
        configInput(PITCH_INPUT, "Pitch");
        // configSwitch(SWITCH_TEST, 0.0f, 1.0f, 1.0f, "Something", {"Value", "Other Value"});
    }

    // Save
    json_t *dataToJson() override
    {
        json_t *json_root = json_object();
        json_object_set_new(json_root, "path", json_string(this->path.c_str()));
        json_object_set_new(json_root, "trig_input_response_mode", json_integer(trig_input_response_mode));
        return json_root;
    }

    // Load
    void dataFromJson(json_t *json_root) override
    {
        json_t *loaded_path_json = json_object_get(json_root, ("path"));
        if (loaded_path_json)
        {
            this->path = json_string_value(loaded_path_json);
            this->load_samples_from_path(this->path.c_str());
        }

        // Load trigger input response mode
        json_t *trig_input_response_mode_json = json_object_get(json_root, "trig_input_response_mode");
        if (trig_input_response_mode_json)
            trig_input_response_mode = json_integer_value(trig_input_response_mode_json);
    }

    void load_samples_from_path(std::string path)
    {
        loading_samples = true;

        // Clear out any old .wav files
        this->sample_players.clear();

        // Load all .wav files found in the folder specified by 'path'
        // this->rootDir = std::string(path);

        std::vector<std::string> dirList = system::getEntries(path.c_str());

        // Sort the vector.  This is in response to a user who's samples were being
        // loaded out of order.  I think it's a mac thing.
        sort(dirList.begin(), dirList.end());

        // TODO: Decide on a maximum memory consuption allowed and abort if
        // that amount of member would be exhausted by loading all of the files
        // in the folder.  Also consider supporting MP3.
        for (auto path : dirList)
        {
            if (
                (rack::string::lowercase(system::getExtension(path)) == "wav") ||
                (rack::string::lowercase(system::getExtension(path)) == ".wav"))
            {
                SamplePlayer sample_player;

                sample_player.loadSample(path);
                this->sample_players.push_back(sample_player);
            }
        }

        loading_samples = false;
    }

    float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
    {
        float input_value = inputs[input_index].getVoltage() / 10.0;
        float knob_value = params[knob_index].getValue();
        float attenuator_value = params[attenuator_index].getValue();

        return (((input_value * scale) * attenuator_value) + (knob_value * scale));
    }

    void process(const ProcessArgs &args) override
    {
        // If the samples are being loaded, don't do anything
        if (loading_samples)
            return;

        unsigned int number_of_samples = sample_players.size();

        // Read the input/knob for sample selection
        unsigned int wav_input_value = calculate_inputs(WAV_INPUT, WAV_KNOB, WAV_ATTN_KNOB, number_of_samples);
        wav_input_value = clamp(wav_input_value, 0, number_of_samples - 1);

        // Read the loop switch
        bool loop = params[LOOP_SWITCH].getValue();

        // Read the pitch input
        float pitch = inputs[PITCH_INPUT].getVoltage();

        //
        // If the sample has been changed.
        //
        if (wav_input_value != selected_sample_slot)
        {
            // Trigger the declick filter if the selected sample has changed
            declick_filter.trigger();

            // Reset sample position so playback does not start at previous sample position
            sample_players[selected_sample_slot].stop();

            // Set the selected sample
            selected_sample_slot = wav_input_value;

            playback = false;
        }

        // Check to see if the selected sample slot refers to an existing sample.
        // If not, return.  This could happen before any samples have been loaded.
        if (!(sample_players.size() > selected_sample_slot))
            return;

        SamplePlayer *selected_sample_player = &sample_players[selected_sample_slot];

        if (inputs[TRIG_INPUT].isConnected())
        {
            if (trig_input_response_mode == TRIGGER)
            {
                // playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
                // determines if a low-to-high transition happened at the TRIG_INPUT

                if (playTrigger.process(inputs[TRIG_INPUT].getVoltage()))
                {
                    playback = true;
                    declick_filter.trigger();
                    selected_sample_player->trigger(SAMPLE_START_POSITION, loop);
                }
            }
            else if (trig_input_response_mode == GATE)
            {
                // In gate mode, continue playing back the sample as long as the gate is high
                // 5.0 volts is required for a gate high signal, but this is extra lenient
                // as the standards say a 10.0 or higher signal should be accepted.
                if (inputs[TRIG_INPUT].getVoltage() >= 5.0)
                {
                    if (playback == false)
                    {
                        playback = true;
                        selected_sample_player->trigger(SAMPLE_START_POSITION, loop); // TODO: May have to come back to this
                        declick_filter.trigger();
                    }
                }
                else
                {
                    playback = false;
                    selected_sample_player->stop();
                    declick_filter.trigger();
                }
            }
        }
        // If no cable is connected to the trigger input, then provide constant playback
        else
        {
            playback = true;
        }

        if (playback)
        {
            float left_audio;
            float right_audio;

            selected_sample_player->getStereoOutput(&left_audio, &right_audio, interpolation);

            declick_filter.process(&left_audio, &right_audio);

            outputs[WAV_LEFT_OUTPUT].setVoltage(left_audio * GAIN);
            outputs[WAV_RIGHT_OUTPUT].setVoltage(right_audio * GAIN);

            selected_sample_player->step(pitch, SAMPLE_START_POSITION, SAMPLE_END_POSITION, loop);
        }
    }
};
