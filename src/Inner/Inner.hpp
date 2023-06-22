// Rename to voxbuilder
#include <fstream>
#include "VoxbuilderLogger.hpp"

struct Inner : VoxglitchModule
{
    float audio_out = 0;
    
    float in1 = 0.0;
    float in2 = 0.0;
    float in3 = 0.0;
    float in4 = 0.0;
    float in5 = 0.0;
    float in6 = 0.0;
    float in7 = 0.0;
    float in8 = 0.0;

    float out1 = 0.0;
    float out2 = 0.0;
    float out3 = 0.0;
    float out4 = 0.0;
    float out5 = 0.0;
    float out6 = 0.0;
    float out7 = 0.0;
    float out8 = 0.0;

    PatchConstructor *patch_constructor = nullptr;
    PatchRunner *patch_runner = nullptr;
    PatchLoader *patch_loader = nullptr;
    Patch *patch = nullptr;

    std::string path = "";
    
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_1,
        INPUT_2,
        INPUT_3,
        INPUT_4,
        INPUT_5,
        INPUT_6,
        INPUT_7,
        INPUT_8,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT_1,
        OUTPUT_2,
        OUTPUT_3,
        OUTPUT_4,
        OUTPUT_5,
        OUTPUT_6,
        OUTPUT_7,
        OUTPUT_8,
        NUM_OUTPUTS
    };

    Inner()
    {
        patch_runner = new PatchRunner();
        patch_loader = new PatchLoader();

        // Create a vector to hold pointers p1 through p8
        std::vector<float*> input_adapters = {&in1, &in2, &in3, &in4, &in5, &in6, &in7, &in8};
        std::vector<float*> output_adapters = {&out1, &out2, &out3, &out4, &out5, &out6, &out7, &out8};

        patch_constructor = new PatchConstructor(input_adapters, output_adapters);
        
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(const ProcessArgs &args) override
    {
        this->audio_out = 0.0;

        // Read and set the input adapter values
        in1 = inputs[INPUT_1].getVoltage();
        in2 = inputs[INPUT_2].getVoltage();
        in3 = inputs[INPUT_3].getVoltage();
        in4 = inputs[INPUT_4].getVoltage();
        in5 = inputs[INPUT_5].getVoltage();
        in6 = inputs[INPUT_6].getVoltage();
        in7 = inputs[INPUT_7].getVoltage();
        in8 = inputs[INPUT_8].getVoltage();

        // TODO: Only update patch_constructor's sample rate when the rate changes

        // Calculate your audio output here
        if (patch_constructor->isReady())
        {
            // audio_out = patch_runner->process(args.sampleRate, patch);
            patch_runner->process(args.sampleRate, patch);
        }

        // Set the outputs to the values of the output adapters
        outputs[OUTPUT_1].setVoltage(out1);
        outputs[OUTPUT_2].setVoltage(out2);
        outputs[OUTPUT_3].setVoltage(out3);
        outputs[OUTPUT_4].setVoltage(out4);
        outputs[OUTPUT_5].setVoltage(out5);
        outputs[OUTPUT_6].setVoltage(out6);
        outputs[OUTPUT_7].setVoltage(out7);
        outputs[OUTPUT_8].setVoltage(out8);
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
