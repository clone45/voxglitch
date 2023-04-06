// TODO: handle feedback and mix

#pragma once
#include "../BaseModule.hpp"
#include "../dsp/AudioDelay.hpp"

class DelayModule : public BaseModule {

private:
    AudioDelay delay;

public:

    enum INPUTS {
        AUDIO,
        DELAY,
        FEEDBACK,
        MIX,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        DELAY_TIME,
        FEEDBACK,
        MIX,
        NUM_PARAMS
    };

    DelayModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        params[DELAY_TIME].setValue(5.0f);
        params[FEEDBACK].setValue(9.0f);
        params[MIX].setValue(9.0f);
    }

    void process(unsigned int sample_rate) override 
    {
        // Get input value
        float input = inputs[AUDIO]->getVoltage();

        // Get parameters
        float delay_time_voltage = inputs[DELAY]->isConnected() ? inputs[DELAY]->getVoltage() : params[DELAY_TIME].getValue();]
        float feedback_voltage = inputs[FEEDBACK]->isConnected() ? inputs[FEEDBACK]->getVoltage() : params[FEEDBACK].getValue();
        float mix_voltage = inputs[MIX]->isConnected() ? inputs[MIX]->getVoltage() : params[MIX].getValue();

        // Normalize voltages to values 0.0 to 1.0
        float delay_time = delay_time_voltage / 10.0f;
        float feedback = feedback_voltage / 10.0f;
        float mix = mix_voltage / 10.0f;

        // Clamp values to [0,1] range
        delay_time = clamp(delay_time, 0.0f, 1.0f);
        feedback = clamp(feedback, 0.0f, 1.0f);
        mix = clamp(mix, 0.0f, 1.0f);

        delay.setDelay(delay_time);

        float output = delay.process(input);

        // Set output value, which will also alert any connected ports
        outputs[OUTPUT]->setValue(output);
    }
};