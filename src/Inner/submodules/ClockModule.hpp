#pragma once
#include "../BaseModule.hpp"
#include "../dsp/SchmittTrigger.hpp"

class ClockModule : public BaseModule {
public:
    enum INPUTS {
        RESET,
        BPM,
        // DIVISION,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        BPM_PARAM,
        NUM_PARAMS
    };

    ClockModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        // params[BPM]->setValue(120.0f); // Default BPM is 120
    }

    void process(unsigned int sample_rate) override 
    {
        // float bpm = params[BPM_PARAM]->getValue();
        float bpm = 120;

        // Clamp BPM to a minimum of 1
        bpm = clamp(bpm, 1.0f, 999.0f);

        float beat_duration = 60.0f / bpm; // Duration of one beat in seconds
        // int division = static_cast<int>(inputs[DIVISION]->getVoltage() + 1); // Clock division factor, minimum of 1
        int division = 1;
        int samples_per_beat = static_cast<int>(sample_rate * beat_duration / division); // Number of samples per beat, divided by division factor

        // If reset input is high, reset sample count
        if (reset_trigger.process(inputs[RESET]->getVoltage())) {
            sample_count = 0;
        }

        // If sample count exceeds number of samples per beat, reset sample count and set output high
        if (sample_count >= samples_per_beat) {
            sample_count = 0;
            outputs[OUTPUT]->setVoltage(10.0f);
        }
        else {
            outputs[OUTPUT]->setVoltage(0.0f);
        }

        sample_count++;       
    }

private:
    int sample_count = 0;
    SchmittTrigger reset_trigger;
};
