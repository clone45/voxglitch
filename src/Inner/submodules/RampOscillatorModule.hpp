#pragma once
#include "../BaseModule.hpp"

class RampOscillatorModule : public BaseModule {
public:
    enum INPUTS {
        FREQUENCY,
        RESET,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        FREQUENCY_PARAM,
        NUM_PARAMS
    };

    RampOscillatorModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override 
    {
        float frequency = inputs[FREQUENCY]->getVoltage();

        if(frequency == 0.0f) 
        {
            outputs[OUTPUT]->setVoltage(0.0f);
            return;
        }

        if(sample_rate == 0) 
        {
            outputs[OUTPUT]->setVoltage(0.0f);
            return;
        }

        float phase_increment = frequency / sample_rate;
        phase += phase_increment;
        
        if (phase >= 1.0f) phase -= 1.0f;

        outputs[OUTPUT]->setVoltage(phase * 10.0f);
    }

private:
    float phase = 0.0f;
};
