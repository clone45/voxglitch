#pragma once
#include "../BaseModule.hpp"

class LinearVCAModule : public BaseModule
{
private:
    float gain = 1.0f;

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

    LinearVCAModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        params[GAIN]->setValue(10.0f);
    }

    void process(unsigned int sample_rate) override
    {
        // Get input values
        float input = inputs[AUDIO]->getVoltage();
        float gain_voltage = inputs[GAIN]->isConnected() ? inputs[GAIN]->getVoltage() : params[GAIN_PARAM]->getValue();
        
        // Clamp gain voltage to [0,1] range
        float gain = clamp(gain_voltage / 10.0, 0.0f, 1.0f);

        // Calculate output value
        float output = input * gain;

        // Set output value, which will also alert any connected ports
        outputs[OUTPUT]->setVoltage(output);
    }
};
