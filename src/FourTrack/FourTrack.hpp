struct FourTrack : VoxglitchSamplerModule
{

    // Create track 1
    TrackModel track1;

    // Create a sample
    Sample sample;

    // Create a clip
    Clip clip1;

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

        sample.load(asset::plugin(pluginInstance, "res/four_track/example.wav"));

        clip1.setSample(&sample);

        track1.addClip(clip1);
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
