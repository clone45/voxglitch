#pragma once
#include "../BaseModule.hpp"

class SampleAndHoldModule : public BaseModule {
public:
    enum INPUTS {
        INPUT,
        HOLD,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        NUM_PARAMS
    };

    SampleAndHoldModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override {
        float input = inputs[INPUT]->getVoltage();
        bool hold = inputs[HOLD]->getVoltage() >= 2.5f;

        static float held_value = 0.0f;

        if (hold) {
            held_value = input;
        }

        float output = hold ? held_value : input;

        outputs[OUTPUT]->setVoltage(output);
    }
};
