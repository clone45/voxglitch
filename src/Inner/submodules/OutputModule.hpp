// OutModule.hpp

#pragma once

#include "../BaseModule.hpp"

class OutputModule : public BaseModule 
{
    float value = 0.0;

    enum INPUTS {
        AUDIO_INPUT,
        NUM_INPUTS
    };

    enum OUTPUTS {
        NUM_OUTPUTS
    };

    enum PARAMS {
        NUM_PARAMS
    };

    OutputModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override 
    {
        value = inputs[AUDIO_INPUT]->getVoltage();
    }     
};