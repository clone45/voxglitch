#pragma once
#include <cmath>
#include "../BaseModule.hpp"

class MoogLowpassFilterModule : public BaseModule
{
public:
    float stages[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float alpha = 0.1f;
    float resonance = 0.0f;

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

    MoogLowpassFilterModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        params[CUTOFF_PARAM]->setValue(10.0f);
        params[RESONANCE_PARAM]->setValue(0.0f);
    }

    void process(unsigned int sample_rate) override
    {
        float x = inputs[AUDIO]->getVoltage();
        
        float cutoff_voltage = inputs[CUTOFF]->isConnected() ? inputs[CUTOFF]->getVoltage() : params[CUTOFF_PARAM]->getValue();
        float resonance_voltage = inputs[RESONANCE]->isConnected() ? inputs[RESONANCE]->getVoltage() : params[RESONANCE_PARAM]->getValue();

        float cutoff_normalized = pow(cutoff_voltage / 10.0f, 2.0f);
        float cutoff_hz = cutoff_normalized * sample_rate / 2.0f;

        float omega = 2.0f * M_PI * cutoff_hz / sample_rate;
        alpha = omega / (omega + 1.0f);

        resonance = clamp(resonance_voltage / 10.0f, 0.0f, 0.99f);

        // Apply the feedback to the input signal
        x -= (resonance * stages[3]) / (1.0f + cutoff_normalized);

        for (int i = 0; i < 4; ++i)
        {
            float output = alpha * (x + stages[i]) + (1.0f - alpha) * stages[i];
            stages[i] = output;
            x = output;
        }

        outputs[OUTPUT]->setVoltage(clamp(stages[3], -20.0f, 20.0f));
    }
};
