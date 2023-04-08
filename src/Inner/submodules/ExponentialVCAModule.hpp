#pragma once
#include <cmath>
#include "../BaseModule.hpp"

class ExponentialVCAModule : public BaseModule
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

    ExponentialVCAModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        params[GAIN_PARAM]->setValue(10.0f);
    }

    void process(unsigned int sample_rate) override
    {
        float gain = 1.0f;
        float input = inputs[AUDIO]->getVoltage();

        if (inputs[GAIN]->isConnected())
        {
            float gain_voltage = inputs[GAIN]->getVoltage();
            gain_voltage = clamp(gain_voltage, -10.0f, 10.0f);
            gain = powf(10.0f, gain_voltage / 20.0f);
        }
        else
        {
            float gain_voltage = params[GAIN_PARAM]->getValue();
            gain_voltage = clamp(gain_voltage, -10.0f, 10.0f);
            gain = gain_voltage / 10.0;
        }

        float output = input * gain;

        outputs[OUTPUT]->setVoltage(output);
    }
};
