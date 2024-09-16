#include "ADPCMEffectsWrapper.hpp"
#include "BlockADPCMProcessor.hpp" // Include this if not already included
#include <rack.hpp>
#include <vector>

struct ConferenceCall : Module
{
    enum ParamIds
    {
        COMPRESSION_PARAM,
        DROPOUT_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        AUDIO_INPUT_LEFT,
        AUDIO_INPUT_RIGHT,
        DUMP_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        AUDIO_OUTPUT_LEFT,
        AUDIO_OUTPUT_RIGHT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        COMPRESSION_LIGHT,
        NUM_LIGHTS
    };

    BlockADPCMProcessor adpcmProcessor;
    ADPCMEffectsWrapper effectsWrapperLeft{adpcmProcessor};
    ADPCMEffectsWrapper effectsWrapperRight{adpcmProcessor};

    AdaptiveGainControl adaptiveGainControl;

    std::vector<float> inputBufferLeft;
    std::vector<float> inputBufferRight;
    std::vector<float> outputBufferLeft;
    std::vector<float> outputBufferRight;

    dsp::SchmittTrigger dumpTrigger;

    bool compressionEnabled;

    ConferenceCall()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(COMPRESSION_PARAM, 0.f, 1.f, 0.f, "ADPCM Compression");
        configParam(DROPOUT_PARAM, 0.f, 1.f, 0.f, "Dropout Amount");

        inputBufferLeft.reserve(BlockADPCMProcessor::BLOCK_SIZE);
        inputBufferRight.reserve(BlockADPCMProcessor::BLOCK_SIZE);

        compressionEnabled = false;

        // Initialize the compression light state
        lights[COMPRESSION_LIGHT].setBrightness(0.0f); // Set initial brightness to off
    }

    json_t *dataToJson() override
    {
        json_t *root = json_object();
        json_object_set_new(root, "compressionEnabled", json_boolean(compressionEnabled));
        return root;
    }

    void dataFromJson(json_t *root) override
    {
        json_t *compressionEnabledJ = json_object_get(root, "compressionEnabled");
        if (compressionEnabledJ)
            compressionEnabled = json_boolean_value(compressionEnabledJ);
    }

    void process(const ProcessArgs &args) override
    {

        float inputLeft = inputs[AUDIO_INPUT_LEFT].getVoltage() / 5.f; // Normalize to [-1, 1]
        float inputRight = inputs[AUDIO_INPUT_RIGHT].getVoltage() / 5.f;

        // Apply adaptive gain
        // adaptiveGainControl.process(inputLeft, inputRight);

        float outputLeft = inputLeft;
        float outputRight = inputRight;

        if (compressionEnabled)
        {
            float dropoutAmount = params[DROPOUT_PARAM].getValue();
            effectsWrapperLeft.setDropoutAmount(dropoutAmount);
            effectsWrapperRight.setDropoutAmount(dropoutAmount);

            // Add inputs to buffer
            if (inputBufferLeft.size() < BlockADPCMProcessor::BLOCK_SIZE) 
            {
                inputBufferLeft.push_back(inputLeft);
                inputBufferRight.push_back(inputRight);
            }

            // Process when input buffer is full
            if (inputBufferLeft.size() == BlockADPCMProcessor::BLOCK_SIZE)
            {
                std::vector<uint8_t> compressedLeft = effectsWrapperLeft.processAndCompress(inputBufferLeft);
                std::vector<uint8_t> compressedRight = effectsWrapperRight.processAndCompress(inputBufferRight);

                std::vector<float> decompressedLeft = effectsWrapperLeft.decompressAndProcess(compressedLeft);
                std::vector<float> decompressedRight = effectsWrapperRight.decompressAndProcess(compressedRight);

                // Append decompressed data to output buffer
                outputBufferLeft.insert(outputBufferLeft.end(), decompressedLeft.begin(), decompressedLeft.end());
                outputBufferRight.insert(outputBufferRight.end(), decompressedRight.begin(), decompressedRight.end());

                // Clear input buffers
                inputBufferLeft.clear();
                inputBufferRight.clear();
            }

            // Use processed output if available
            if (!outputBufferLeft.empty() && !outputBufferRight.empty())
            {
                outputLeft = outputBufferLeft.front();
                outputRight = outputBufferRight.front();
                outputBufferLeft.erase(outputBufferLeft.begin());
                outputBufferRight.erase(outputBufferRight.begin());
            }

            // Update the compression light to indicate it's active
            lights[COMPRESSION_LIGHT].setBrightness(1.0f); // Set brightness to on
        }
        else
        {
            // Turn off the compression light
            lights[COMPRESSION_LIGHT].setBrightness(0.0f); // Set brightness to off
        }

        outputs[AUDIO_OUTPUT_LEFT].setVoltage(outputLeft * 5.f); // Denormalize to [-5, 5]
        outputs[AUDIO_OUTPUT_RIGHT].setVoltage(outputRight * 5.f);
    }
};
