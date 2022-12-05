//
// Kodama - Performance sampler with sliders

struct Kodama : VoxglitchSamplerModule
{
    SamplePlayer sample_players[NUMBER_OF_SLIDERS];

    enum ParamIds
    {
        ENUMS(SLIDERS, NUMBER_OF_SLIDERS),
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        // ENUMS(OUTPUTS, NUMBER_OF_SLIDERS * 2),
        AUDIO_OUTPUT_LEFT,
        AUDIO_OUTPUT_RIGHT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Kodama()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    //
    // Autosave module data.  VCV Rack decides when this should be called.
    //
    // My saving and loading code could be more compact by saving arrays of
    // "ball" data tidily packaged up.  I'll do that if this code ever goes
    // through another iteration.
    //

    json_t *dataToJson() override
    {
        json_t *json_root = json_object();

        return json_root;
    }

    // Load module data
    void dataFromJson(json_t *json_root) override
    {
    }

    void process(const ProcessArgs &args) override
    {
        float mix_output_left = 0;
        float mix_output_right = 0;

        for(unsigned int slider_index = 0; slider_index < NUMBER_OF_SLIDERS; slider_index++)
        {
            float left_audio = 0;
            float right_audio = 0;
            this->sample_players[slider_index].getStereoOutput(&left_audio, &right_audio, interpolation);

            float volume = params[SLIDERS + slider_index].getValue();
            
            mix_output_left += (volume * left_audio);
            mix_output_right += (volume * right_audio);

// step(float pitch = 0.0, float sample_start = 0.0, float sample_end = 1.0, bool loop = false)

            float pitch = 0.0;
            float sample_start = 0.0;
            float sample_end = 1.0;
            bool loop = true;

            this->sample_players[slider_index].step(pitch, sample_start, sample_end, loop);
        }

        outputs[AUDIO_OUTPUT_LEFT].setVoltage(mix_output_left);
        outputs[AUDIO_OUTPUT_RIGHT].setVoltage(mix_output_right);        
    }
};
