// Rename to voxbuilder
struct Inner : VoxglitchModule
{
    float audio_out = 0;
    float p1 = 0.0;
    float p2 = 0.0;
    float p3 = 0.0;
    float pitch = 0.0;
    float gate = 0.0;
    
    ModuleManager *module_manager;

    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        PITCH_INPUT,
        GATE_INPUT,
        PARAM1_CV_INPUT,
        PARAM2_CV_INPUT,
        PARAM3_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    Inner()
    {
        module_manager = new ModuleManager(&pitch, &gate, &p1, &p2, &p3);
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(const ProcessArgs &args) override
    {
        float audio_out = 1.0;

        p1 = inputs[PARAM1_CV_INPUT].getVoltage();
        p2 = inputs[PARAM2_CV_INPUT].getVoltage();
        p3 = inputs[PARAM3_CV_INPUT].getVoltage();
        pitch = inputs[PITCH_INPUT].getVoltage();
        gate = inputs[GATE_INPUT].getVoltage();

        // Calculate your audio output here
        if(module_manager->isReady()) audio_out = module_manager->process();        
        
        // Set the output value
        outputs[AUDIO_OUTPUT].setVoltage(audio_out);
    }
};
