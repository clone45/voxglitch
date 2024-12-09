struct CueResearchExpander : VoxglitchModule
{
    std::string loaded_filename = "[ EMPTY ]";

    enum ParamIds {
        ENUMS(MARKER_SELECTORS, 32),  // Match main module's selector structure
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(MARKER_INPUTS, 32),     // 32 trigger inputs matching main outputs
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(MARKER_SELECTOR_LIGHTS, 32),  // Lights for the selectors
        NUM_LIGHTS
    };

    // Constructor

    CueResearchExpander() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        
        // Configure the parameters and inputs
        for (int i = 0; i < 32; i++) {
            configParam(MARKER_SELECTORS + i, 0.f, 1.f, 0.f, "Marker Input Selector " + std::to_string(i + 1));
            configInput(MARKER_INPUTS + i, "Marker Input " + std::to_string(i + 1));
        }
    }


    // █▀ ▄▀█ █░█ █▀▀   ▄▀█ █▄░█ █▀▄   █░░ █▀█ ▄▀█ █▀▄
    // ▄█ █▀█ ▀▄▀ ██▄   █▀█ █░▀█ █▄▀   █▄▄ █▄█ █▀█ █▄▀

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
    }



    // █▀█ █▀█ █▀█ █▀▀ █▀▀ █▀ █▀
    // █▀▀ █▀▄ █▄█ █▄▄ ██▄ ▄█ ▄█

    void process(const ProcessArgs &args) override
    {

    }
};
