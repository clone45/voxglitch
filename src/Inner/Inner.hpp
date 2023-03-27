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
        float audio_out = 1.0;

        // Calculate your audio output here
        if(module_manager.isReady()) audio_out = module_manager.process();        
        
        // Set the output value
        outputs[AUDIO_OUTPUT].setVoltage(audio_out);
    }
};
