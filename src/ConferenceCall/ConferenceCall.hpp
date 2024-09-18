#include "ADPCMEffectsWrapper.hpp"
#include "BlockADPCMProcessor.hpp"
#include "UDPNetworkSimulator.hpp"
#include <rack.hpp>
#include <vector>

struct ConferenceCall : Module
{
    enum ParamIds
    {
        COMPRESSION_PARAM,
        DROPOUT_PARAM,
        LATENCY_PARAM,
        JITTER_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        AUDIO_INPUT_LEFT,
        AUDIO_INPUT_RIGHT,
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
    ADPCMEffectsWrapper adpcmWrapper{adpcmProcessor};
    UDPNetworkSimulator udp{48000, 20};  // 48 kHz sample rate, 20ms frames

    std::vector<float> inputBufferLeft;
    std::vector<float> inputBufferRight;
    std::vector<float> outputBufferLeft;
    std::vector<float> outputBufferRight;

    bool compressionEnabled;

    ConferenceCall()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(COMPRESSION_PARAM, 0.f, 1.f, 0.f, "ADPCM Compression");
        configParam(DROPOUT_PARAM, 0.f, 1.f, 0.f, "Dropout Amount");
        configParam(LATENCY_PARAM, 0.f, 500.f, 0.f, "Latency (ms)");
        configParam(JITTER_PARAM, 0.f, 1.f, 0.f, "Jitter Amount");

        inputBufferLeft.reserve(BlockADPCMProcessor::BLOCK_SIZE);
        inputBufferRight.reserve(BlockADPCMProcessor::BLOCK_SIZE);

        compressionEnabled = false;
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

        float outputLeft = inputLeft;
        float outputRight = inputRight;

        // Add inputs to buffer
        inputBufferLeft.push_back(inputLeft);
        inputBufferRight.push_back(inputRight);

        if (compressionEnabled && inputBufferLeft.size() == BlockADPCMProcessor::BLOCK_SIZE)
        {
            std::vector<uint8_t> compressedLeft = adpcmWrapper.compress(inputBufferLeft);
            std::vector<uint8_t> compressedRight = adpcmWrapper.compress(inputBufferRight);

            udp.setDropoutRate(params[DROPOUT_PARAM].getValue());
            udp.setLatency(params[LATENCY_PARAM].getValue());
            udp.setJitter(params[JITTER_PARAM].getValue());

            udp.post(compressedLeft, compressedRight);
            udp.process();

            std::pair<std::vector<uint8_t>, std::vector<uint8_t>> received = udp.get();
            std::vector<uint8_t>& receivedLeft = received.first;
            std::vector<uint8_t>& receivedRight = received.second;

            if (!receivedLeft.empty() && !receivedRight.empty()) {
                std::vector<float> decompressedLeft = adpcmWrapper.decompress(receivedLeft);
                std::vector<float> decompressedRight = adpcmWrapper.decompress(receivedRight);

                // Append decompressed data to output buffer
                outputBufferLeft.insert(outputBufferLeft.end(), decompressedLeft.begin(), decompressedLeft.end());
                outputBufferRight.insert(outputBufferRight.end(), decompressedRight.begin(), decompressedRight.end());
            }

            // Clear input buffers
            inputBufferLeft.clear();
            inputBufferRight.clear();
        }

        // Use processed output if available, otherwise use input directly
        if (!outputBufferLeft.empty() && !outputBufferRight.empty())
        {
            outputLeft = outputBufferLeft.front();
            outputRight = outputBufferRight.front();
            outputBufferLeft.erase(outputBufferLeft.begin());
            outputBufferRight.erase(outputBufferRight.begin());
        }

        lights[COMPRESSION_LIGHT].setBrightness(compressionEnabled ? 1.0f : 0.0f);

        outputs[AUDIO_OUTPUT_LEFT].setVoltage(outputLeft * 5.f); // Denormalize to [-5, 5]
        outputs[AUDIO_OUTPUT_RIGHT].setVoltage(outputRight * 5.f);
    }
};