#pragma once
#include "../BaseModule.hpp"

class SubtractionModule : public BaseModule {
public:
    enum INPUTS {
        IN1,
        IN2,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    SubtractionModule() 
    {
        config(0, NUM_INPUTS, NUM_OUTPUTS); // No parameters for this module
    }

    void process(unsigned int sample_rate) override 
    {
        float in1 = inputs[IN1]->getVoltage();
        float in2 = inputs[IN2]->getVoltage();

        outputs[OUTPUT]->setVoltage(in2 - in1);
    }
};
