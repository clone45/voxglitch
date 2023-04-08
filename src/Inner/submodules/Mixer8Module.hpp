#pragma once
#include <vector>
#include "../BaseModule.hpp"

class Mixer8Module : public BaseModule {
private:
    enum INPUTS {
        INPUT_1,
        INPUT_2,
        INPUT_3,
        INPUT_4,
        INPUT_5,
        INPUT_6,
        INPUT_7,
        INPUT_8,
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
    Mixer8Module()
    {
        config(NUMBER_OF_PARAMS, NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS);
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
