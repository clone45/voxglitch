// TODO: Add additional waveforms

#pragma once
#include <cmath>
#include "../BaseModule.hpp"

class LFOModule : public BaseModule
{
    public:
    
    float phase = 0.0f;

    enum INPUTS {
        FREQUENCY,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        FREQUENCY_PARAM,
        NUM_PARAMS
    };

    LFOModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        params[FREQUENCY]->setValue(5.0f);
    }

    void process(unsigned int sample_rate) override
    {
        float frequency = inputs[FREQUENCY]->isConnected() ? inputs[FREQUENCY]->getVoltage() : params[FREQUENCY_PARAM]->getValue();
        float phase_increment = frequency / sample_rate;

        // Update phase
        phase += phase_increment;
        if (phase >= 1.0f) phase -= 1.0f;

        // Calculate output
        float out = sinf(phase * 2.0f * 3.14159265358979323846264338327950288);

        // Scale output to ±5V range
        out *= 5.0f;

        // Set output value, which will also alert any connected ports
        outputs[OUTPUT]->setVoltage(out);
    }
};
