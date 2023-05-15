#pragma once
#include "../BaseModule.hpp"

class AmplifierModule : public BaseModule
{
public:

    enum INPUTS {
        AUDIO,
        GAIN,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        GAIN_PARAM,
        NUM_PARAMS
    };

    AmplifierModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        // Get the audio input
        float x = inputs[AUDIO]->getVoltage();

        // Get the gain parameter or input
        float gain = inputs[GAIN]->isConnected() ? inputs[GAIN]->getVoltage() : params[GAIN_PARAM]->getValue();

        // Apply the gain
        float y = gain * x;

        // Set the output
        outputs[OUTPUT]->setVoltage(y);
    }
};
