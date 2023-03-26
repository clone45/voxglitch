struct Inner : VoxglitchModule
{
    float audio_out = 0;
    ModuleManager module_manager;
    
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    Inner()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(const ProcessArgs &args) override
    {
        // Calculate your audio output here
        float audio_out = 0.0f;

        // Set the output value
        outputs[AUDIO_OUTPUT].setVoltage(audio_out);
    }
};
