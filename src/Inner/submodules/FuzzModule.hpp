#pragma once

#include <cmath>
#include "../BaseModule.hpp"

class FuzzModule : public BaseModule 
{

public:

    enum INPUTS {
        AUDIO,
        FUZZ,
        TONE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        FUZZ_PARAM,
        TONE_PARAM,
        NUM_PARAMS
    };

    FuzzModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        float input = inputs[AUDIO]->isConnected() ? inputs[AUDIO]->getVoltage() : 0.0f;
        float fuzz = inputs[FUZZ]->isConnected() ? inputs[FUZZ]->getVoltage() : params[FUZZ_PARAM]->getValue();
        float tone = inputs[TONE]->isConnected() ? inputs[TONE]->getVoltage() : params[TONE_PARAM]->getValue();

        float fuzzedSignal = fuzzEffect(input, fuzz);
        float output = toneControl(fuzzedSignal, tone);

        outputs[OUTPUT]->setVoltage(output);
    }

private:
    float fuzzEffect(float input, float fuzz) 
    {
        float gain = 1.0f + 10.0f * fuzz;
        return gain * input * input / (1.0f + std::abs(gain * input));
    }

    float toneControl(float input, float tone)
    {
        float lpf = 0.5f * (1.0f - tone);
        float hpf = 0.5f * (1.0f + tone);
        float lowPassOutput = lpf * input + (1.0f - lpf) * input;
        float highPassOutput = hpf * input - (1.0f - hpf) * input;
        return lowPassOutput + highPassOutput;
    }
};
