#pragma once

#include <cmath>
#include "../BaseModule.hpp"

class WaveFolderModule : public BaseModule 
{

public:

    enum INPUTS {
        AUDIO,
        FOLD,
        SYMMETRY,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        FOLD_PARAM,
        SYMMETRY_PARAM,
        NUM_PARAMS
    };

    WaveFolderModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        float input = inputs[AUDIO]->isConnected() ? inputs[AUDIO]->getVoltage() : 0.0f;
        float fold = inputs[FOLD]->isConnected() ? inputs[FOLD]->getVoltage() : params[FOLD_PARAM]->getValue();
        float symmetry = inputs[SYMMETRY]->isConnected() ? inputs[SYMMETRY]->getVoltage() : params[SYMMETRY_PARAM]->getValue();

        float output = waveFolder(input, fold, symmetry);
        outputs[OUTPUT]->setVoltage(output);
    }

private:
    float waveFolder(float input, float fold, float symmetry) 
    {
        float foldedSignal = input;
        float threshold = 1.0f - fold;

        if (input > threshold) {
            foldedSignal = (1.0f - symmetry) * (1.0f - input);
        } else if (input < -threshold) {
            foldedSignal = (-1.0f + symmetry) * (1.0f + input);
        }

        return foldedSignal;
    }
};
