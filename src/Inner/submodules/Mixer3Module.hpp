#pragma once
#include <vector>
#include "../BaseModule.hpp"

class Mixer3Module : public BaseModule {
private:
    enum INPUTS {
        INPUT_1,
        INPUT_2,
        INPUT_3,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        NUM_PARAMS
    };

public:
    Mixer3Module()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override 
    {
        float mix_value = 0.0f;
        for (auto input : inputs) 
        {
            if (input->isConnected()) 
            {
                mix_value += input->getVoltage();
            }
        }
        outputs[OUTPUT]->setVoltage(mix_value / 8.0f);
    }
};
