struct FourTrack : VoxglitchSamplerModule
{

    // Create track 1
    TrackModel track1;

    // Create a sample
    Sample sample;

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

    FourTrack()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        sample.load("e:/dev/example.wav");
        track1.setSample(&sample);
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
    }
};
