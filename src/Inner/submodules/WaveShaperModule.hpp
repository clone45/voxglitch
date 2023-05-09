#pragma once

#include <cmath>
#include "../BaseModule.hpp"

class WaveShaperModule : public BaseModule 
{

public:

    enum INPUTS {
        IN,
        CURVE,
        DRIVE,
        BIAS,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        CURVE_PARAM,
        DRIVE_PARAM,
        BIAS_PARAM,
        NUM_PARAMS
    };

    WaveShaperModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        float input = inputs[IN]->isConnected() ? inputs[IN]->getVoltage() : 0.0f;
        float curve = inputs[CURVE]->isConnected() ? inputs[CURVE]->getVoltage() : params[CURVE_PARAM]->getValue();
        float drive = inputs[DRIVE]->isConnected() ? inputs[DRIVE]->getVoltage() : params[DRIVE_PARAM]->getValue();
        float bias = inputs[BIAS]->isConnected() ? inputs[BIAS]->getVoltage() : params[BIAS_PARAM]->getValue();

        float output = waveShaper(input, curve, drive, bias);
        outputs[OUTPUT]->setVoltage(output);
    }

private:

    float fastTanh(float x)
    {
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    float waveShaper(float input, float curve, float drive, float bias) 
    {
        input = input * drive + bias;

        if (curve > 0.0f) {
            input = input * (1.0f - curve) + fastTanh(curve * input) * curve;
        } else {
            input = input * (1.0f + curve) - fastTanh(-curve * input) * curve;
        }

        return input;
    }
};
