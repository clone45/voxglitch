//
// Kodama - Performance sampler with sliders

struct Kodama : VoxglitchModule
{
    bool dirty = false;
    std::string text;
    dsp::BooleanTrigger step_trigger;

    // bool grid_data[COLS][ROWS];

    GateSequencer gate_sequencers[ROWS];

    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        STEP_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Kodama()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (unsigned int row = 0; row < ROWS; row++)
        {
            gate_sequencers[row].assign(32, 0.0);
            gate_sequencers[row].setLength(32);
        }
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
        if(step_trigger.process(inputs[STEP_INPUT].getVoltage()))
        {
            // Step all of the gate sequencers
            for(unsigned int i=0; i < COLS; i++)
            {
                gate_sequencers[i].step();
                // if(trigger_results[i]) gateOutputPulseGenerators[i].trigger(0.01f);
            }
        }        
    }
};
