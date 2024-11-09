struct CueResearchExpander : VoxglitchModule
{
    std::string loaded_filename = "[ EMPTY ]";

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
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };



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
