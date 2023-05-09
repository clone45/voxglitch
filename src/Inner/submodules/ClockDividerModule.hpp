#pragma once
#include "../BaseModule.hpp"

class ClockDividerModule : public BaseModule {
public:
    enum INPUTS {
        INPUT,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUT,
        D2,
        D3,
        D4,
        D5,
        D6,
        D7,
        D8,
        NUM_OUTPUTS
    };

    ClockDividerModule() {
        config(0, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override {
        float input = inputs[INPUT]->getVoltage();

        outputs[OUT]->setVoltage(input);

        outputs[D2]->setVoltage(input);
        outputs[D3]->setVoltage(input);
        outputs[D4]->setVoltage(input);
        outputs[D5]->setVoltage(input);
        outputs[D6]->setVoltage(input);
        outputs[D7]->setVoltage(input);
        outputs[D8]->setVoltage(input);

        if (sample_count % 2 == 0) {
            outputs[D2]->setVoltage(0.0f);
        }
        if (sample_count % 3 == 0) {
            outputs[D3]->setVoltage(0.0f);
        }
        if (sample_count % 4 == 0) {
            outputs[D4]->setVoltage(0.0f);
        }
        if (sample_count % 5 == 0) {
            outputs[D5]->setVoltage(0.0f);
        }
        if (sample_count % 6 == 0) {
            outputs[D6]->setVoltage(0.0f);
        }
        if (sample_count % 7 == 0) {
            outputs[D7]->setVoltage(0.0f);
        }
        if (sample_count % 8 == 0) {
            outputs[D8]->setVoltage(0.0f);
        }

        sample_count++;
    }

private:
    int sample_count = 0;
};