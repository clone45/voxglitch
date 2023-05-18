// Rename to voxbuilder
#include <fstream>
#include "VoxbuilderLogger.hpp"

struct Inner : VoxglitchModule
{
    float audio_out = 0;
    float p1 = 0.0;
    float p2 = 0.0;
    float p3 = 0.0;
    float p4 = 0.0;
    float p5 = 0.0;
    float p6 = 0.0;
    float p7 = 0.0;
    float p8 = 0.0;

    float pitch = 0.0;
    float gate = 0.0;

    PatchConstructor *patch_constructor = nullptr;
    PatchRunner *patch_runner = nullptr;
    PatchLoader *patch_loader = nullptr;
    Patch *patch = nullptr;

    IModule *terminal_output_module = nullptr;
    std::string path = "";
    
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
        PARAM4_CV_INPUT,
        PARAM5_CV_INPUT,
        PARAM6_CV_INPUT,
        PARAM7_CV_INPUT,
        PARAM8_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    Inner()
    {
        patch_runner = new PatchRunner();
        patch_loader = new PatchLoader();
        patch_constructor = new PatchConstructor(&pitch, &gate, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8);
        
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(const ProcessArgs &args) override
    {
        float audio_out = 1.0;

        p1 = inputs[PARAM1_CV_INPUT].getVoltage();
        p2 = inputs[PARAM2_CV_INPUT].getVoltage();
        p3 = inputs[PARAM3_CV_INPUT].getVoltage();
        p4 = inputs[PARAM4_CV_INPUT].getVoltage();
        p5 = inputs[PARAM5_CV_INPUT].getVoltage();
        p6 = inputs[PARAM6_CV_INPUT].getVoltage();
        p7 = inputs[PARAM7_CV_INPUT].getVoltage();
        p8 = inputs[PARAM8_CV_INPUT].getVoltage();

        pitch = inputs[PITCH_INPUT].getVoltage();
        gate = inputs[GATE_INPUT].getVoltage();

        // TODO: Only update patch_constructor's sample rate when the rate changes

        // Calculate your audio output here
        if (patch_constructor->isReady())
        {
            // audio_out = module_manager->process(args.sampleRate);
            audio_out = patch_runner->process(args.sampleRate, terminal_output_module);
        }

        // Set the output value
        outputs[AUDIO_OUTPUT].setVoltage(audio_out);
    }

    void load_config_file(const std::string& filename)
    {
        // set module_manager ready to false
        patch_constructor->setReady(false);

        json_t* root = patch_loader->load(filename);

        if (root == nullptr)
        {
            VoxbuilderLogger::getInstance().log("Failed to load patch.");
            return;
        }

        //
        /////////////////////////////////////////////

        this->patch = patch_constructor->createPatch(root);

        /////////////////////////////////////////////
        //

        if (this->patch == nullptr)
        {
            VoxbuilderLogger::getInstance().log("Failed to create patch.");
            return;
        }

        json_decref(root);

        this->terminal_output_module = patch->getTerminalOutputModule();

        patch_constructor->setReady(true);
    }


    std::string selectFileVCV()
    {
        std::string filename_string = "";
        osdialog_filters *filters = osdialog_filters_parse("JSON:json");
        char *filename = osdialog_file(OSDIALOG_OPEN, "", NULL, filters);

        if (filename != NULL)
        {
            filename_string.assign(filename);
            osdialog_filters_free(filters);
            std::free(filename);
        }

        return (filename_string);
    }    
};
