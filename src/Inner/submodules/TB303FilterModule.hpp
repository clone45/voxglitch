#pragma once

#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"

class TB303FilterModule : public BaseModule {

private:
    float buf0 = 0.0f;
    float buf1 = 0.0f;
    float buf2 = 0.0f;
    float buf3 = 0.0f;

public:

    enum INPUTS {
        AUDIO,
        CUTOFF,
        RESONANCE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        CUTOFF_PARAM,
        RESONANCE_PARAM,
        NUM_PARAMS
    };

    TB303FilterModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        params[CUTOFF]->setValue(0.5f);
        params[RESONANCE]->setValue(0.5f);
    }

    void process(unsigned int sample_rate) override 
    {
        float input = inputs[AUDIO]->isConnected() ? inputs[AUDIO]->getVoltage() : 0.0f;
        float cutoff = inputs[CUTOFF]->isConnected() ? (inputs[CUTOFF]->getVoltage() / 5.0) : params[CUTOFF_PARAM]->getValue();
        float resonance = inputs[RESONANCE]->isConnected() ? (inputs[RESONANCE]->getVoltage() / 1.0) : params[RESONANCE_PARAM]->getValue();

        float fc = 110.0f * powf(2.0f, 3.0f * cutoff);
        float g = tanf(M_PI * fc / sample_rate);
        float r = resonance * (1.2f + 0.5f * g / (g + 1.0f));

        float input_t = input - r * buf3;

        // Diode ladder filter design (four one-pole low-pass filters in series)
        buf0 += g * (tanhf(input_t) - tanhf(buf0));
        buf1 += g * (tanhf(buf0) - tanhf(buf1));
        buf2 += g * (tanhf(buf1) - tanhf(buf2));
        buf3 += g * (tanhf(buf2) - tanhf(buf3));

        outputs[OUTPUT]->setVoltage(buf3 * 5.0f);
    }   
};