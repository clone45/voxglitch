#pragma once
#include "../BaseModule.hpp"

class SequencerModule : public BaseModule {
public:
    enum INPUTS {
        CLOCK,
        RESET,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        SEQUENCE,
        NUM_PARAMS
    };

    SequencerModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override 
    {
        float clock = inputs[CLOCK]->getVoltage();
        float reset = inputs[RESET]->getVoltage();

    }

private:
    int current_step = 0;
};