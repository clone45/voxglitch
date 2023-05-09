#pragma once

#include <cmath>
#include "../BaseModule.hpp"

class OverdriveModule : public BaseModule 
{

public:

    enum INPUTS {
        AUDIO,
        DRIVE,
        TONE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        DRIVE_PARAM,
        TONE_PARAM,
        NUM_PARAMS
    };

    OverdriveModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        float input = inputs[AUDIO]->isConnected() ? inputs[AUDIO]->getVoltage() : 0.0f;
        float drive = inputs[DRIVE]->isConnected() ? (inputs[DRIVE]->getVoltage() / 10.0)  : params[DRIVE_PARAM]->getValue();
        float tone = inputs[TONE]->isConnected() ? (inputs[TONE]->getVoltage() / 10.0) : params[TONE_PARAM]->getValue();

        float output = overdrive(input, drive);
        output = toneControl(output, tone, sample_rate);
        outputs[OUTPUT]->setVoltage(output);
    }

private:
    float overdrive(float input, float drive) 
    {
        drive = clamp(drive, 0.0f, 0.99f);
        float k = 2 * drive / (1 - drive);
        return (1 + k) * input / (1 + k * std::abs(input));
    }

    float toneControl(float input, float tone, unsigned int sample_rate)
    {
        float alpha = (1 - tone) / (1 + tone);
        float rc = 1.0f / (2.0f * M_PI * 1000.0f);
        float dt = 1.0f / static_cast<float>(sample_rate);
        float a = rc / (rc + dt);

        static float y1 = 0.0f;
        static float x1 = 0.0f;

        float lpf = a * y1 + (1 - a) * input;
        float hpf = alpha * (y1 + input - x1);

        x1 = input;
        y1 = lpf;

        return lpf + hpf;
    }
};
