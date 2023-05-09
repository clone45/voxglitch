#pragma once

#include "../BaseModule.hpp"

class Selector3Module : public BaseModule
{
public:
    enum INPUTS {
        IN1,
        IN2,
        IN3,
        SEL,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        NUM_PARAMS
    };

    Selector3Module()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        float selection_voltage = inputs[SEL]->getVoltage();
        selection_voltage = clamp(selection_voltage, 0.0f, 10.0f);

        float selected_input = 0.0f;

        switch (static_cast<int>((selection_voltage / 10.0f) * NUM_INPUTS)) {
            case 0:
                selected_input = inputs[IN1]->getVoltage();
                break;
            case 1:
                selected_input = inputs[IN2]->getVoltage();
                break;
            case 2:
                selected_input = inputs[IN3]->getVoltage();
                break;
            default:
                selected_input = inputs[IN1]->getVoltage();
                break;
        }

        outputs[OUTPUT]->setVoltage(selected_input);
    }
};
