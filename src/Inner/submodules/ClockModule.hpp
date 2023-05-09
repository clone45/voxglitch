#pragma once
#include "../BaseModule.hpp"

class ClockModule : public BaseModule {
public:
    enum INPUTS {
        RESET,
        DIVISION,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        BPM,
        NUM_PARAMS
    };

    ClockModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        params[BPM]->setValue(120.0f); // Default BPM is 120
    }

    void process(unsigned int sample_rate) override 
    {
        float bpm = params[BPM]->getValue();
        float beat_duration = 60.0f / bpm; // Duration of one beat in seconds
        int division = static_cast<int>(inputs[DIVISION]->getVoltage() + 1); // Clock division factor, minimum of 1
        int samples_per_beat = static_cast<int>(sample_rate * beat_duration / division); // Number of samples per beat, divided by division factor

        // Check if reset input is high, reset sample counter if it is
        if (inputs[RESET]->getVoltage() > 0.0f) {
            sample_count = 0;
        }

        // Check if enough samples have elapsed to generate a clock pulse
        if (sample_count >= samples_per_beat) {
            outputs[OUTPUT]->setVoltage(10.0f); // Output high voltage
            sample_count = 0; // Reset sample counter
        } else {
            outputs[OUTPUT]->setVoltage(0.0f); // Output low voltage
        }

        sample_count++;
    }

private:
    int sample_count = 0;
};
