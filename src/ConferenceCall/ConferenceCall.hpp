#include "VoIPSimulator.hpp"
#include <rack.hpp>

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

    VoIPSimulator voip;
    bool compressionEnabled;

    ConferenceCall()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(COMPRESSION_PARAM, 0.f, 1.f, 0.f, "ADPCM Compression");
        configParam(DROPOUT_PARAM, 0.f, 1.f, 0.f, "Dropout Amount");
        configParam(LATENCY_PARAM, 0.f, 500.f, 0.f, "Latency (ms)");
        configParam(JITTER_PARAM, 0.f, 1.f, 0.f, "Jitter Amount");

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

        if (compressionEnabled)
        {
            voip.setNetworkParams(
                params[DROPOUT_PARAM].getValue(),
                params[LATENCY_PARAM].getValue(),
                params[JITTER_PARAM].getValue()
            );

            voip.pushSamples(inputLeft, inputRight);
            voip.process();

            if (voip.hasOutput()) {
                std::tie(outputLeft, outputRight) = voip.popOutputSamples();
            }
        }

        lights[COMPRESSION_LIGHT].setBrightness(compressionEnabled ? 1.0f : 0.0f);

        outputs[AUDIO_OUTPUT_LEFT].setVoltage(outputLeft * 5.f); // Denormalize to [-5, 5]
        outputs[AUDIO_OUTPUT_RIGHT].setVoltage(outputRight * 5.f);
    }
};