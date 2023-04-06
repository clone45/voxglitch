#pragma once

#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"

class TB303OscillatorModule : public BaseModule 
{
    float phase = 0.0f;
    float last_value = 0.0f;
    float accent = 1.0f;

    enum INPUTS {
        FREQUENCY,
        RESONANCE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        FREQUENCY,
        RESONANCE,
        NUM_PARAMS
    };

    TB303OscillatorModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        params[FREQUENCY].setValue(440.0f);
        params[RESONANCE].setValue(0.0f);
    }

    void process(unsigned int sample_rate) override 
    {
        float frequency = params[FREQUENCY].getValue();

        if (frequency_input_port->isConnected()) 
        {
            float voltage = frequency_input_port->getVoltage();
            frequency = 8.18f * powf(2.0f, voltage);
        }

        // Get the resonance value.  If the resonance input port is connected, use
        // that value.  Otherwise, use the resonance parameter value.
        float resonance = inputs[RESONANCE]->isConnected() ? inputs[RESONANCE]->getVoltage() : params[RESONANCE].getValue();

        // TB-303-style exponential frequency scaling
        frequency = 0.1f * powf(2.0f, 3.0f * frequency) * sample_rate;

        // Calculate the phase increment and update the phase
        float phase_increment = frequency / sample_rate;
        phase += phase_increment;

        // Ensure that the phase stays between 0 and 1
        if (phase >= 1.0f) phase -= 1.0f;

        // Calculate the sawtooth waveform
        float saw = phase * 2.0f - 1.0f;

        // Calculate the square waveform
        float square = (saw >= 0.0f) ? 1.0f : -1.0f;

        // TB-303-style resonance
        float feedback = resonance + (resonance / (1.0f - last_value * last_value));
        float input = square + feedback * last_value;
        last_value = input;

        // Calculate the output
        float output = (1.0f - accent) * saw + accent * input;

        // Set output value, which will also alert any connected ports
        outputs[OUTPUT]->setVoltage(output * 5.0f);
    }
};
