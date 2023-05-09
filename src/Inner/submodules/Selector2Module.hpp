#pragma once

#include "../BaseModule.hpp"

class Selector2Module : public BaseModule
{
public:
    enum INPUTS {
        IN1,
        IN2,
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

    Selector2Module()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        float sel_voltage = inputs[SEL]->getVoltage();
        float input_signal = 0.0f;

        switch (int(sel_voltage / 5.0f)) {
            case 0:
                input_signal = inputs[IN1]->getVoltage();
                break;
            case 1:
                input_signal = inputs[IN2]->getVoltage();
                break;
            default:
                // Invalid input voltage, default to IN1
                input_signal = inputs[IN1]->getVoltage();
                break;
        }

        outputs[OUTPUT]->setVoltage(input_signal);
    }
};
