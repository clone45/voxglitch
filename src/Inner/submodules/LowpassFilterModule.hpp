#pragma once
#include <cmath>
#include "../BaseModule.hpp"

class LowpassFilterModule : public BaseModule
{
    float y = 0.0f;  // output of the filter
    float alpha = 0.1f;  // filter coefficient
    float resonance = 0.0f; // resonance amount

    enum INPUTS {
        AUDIO,
        CUTOFF,
        RESONANCE,
        NUMBER_OF_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUMBER_OF_OUTPUTS
    };

    enum PARAMS {
        NUMBER_OF_PARAMS
    };

    LowpassFilterModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        params[CUTOFF].setValue(10.0f);
        params[RESONANCE].setValue(0.0f);
    }

    void process(unsigned int sample_rate) override
    {
        // Get the audio input
        float x = inputs[AUDIO]->getVoltage();

        float cutoff_voltage = inputs[CUTOFF]->isConnected() ? cutoff_input_port->getVoltage() : params[CUTOFF].getValue();
        float resonance_voltage = inputs[RESONANCE]->isConnected() ? resonance_input_port->getVoltage() : params[RESONANCE].getValue();

        // Adjust the exponent to control the degree to which the response is
        // accentuated in the lower range of cutoff voltages. Higher exponents
        // create a more subtle response in the lower range, while lower exponents
        // create a more pronounced response. A value of around 2 seems to work
        // well for most audio applications.
        float exponent = 4.0f;

        float cutoff_normalized = pow(cutoff_voltage / 10.0f, exponent);

        // Calculate the cutoff frequency in Hz
        float cutoff_hz = cutoff_normalized * sample_rate / 2.0f;

        // Calculate the filter coefficient
        float omega = 2.0f * 3.14159265358979323846 * cutoff_hz / sample_rate;
        alpha = omega / (omega + 1.0f);

        // Scale the resonance amount to be between 0 and 1
        resonance = clamp(resonance_voltage / 10.0f, 0.0f, 0.99f);

        // Calculate the feedback coefficient based on the cutoff frequency and resonance parameter
        float k = 2.0f * std::sin(3.14159265358979323846 * cutoff_hz / sample_rate);
        float feedback = resonance * k / (1.0f - alpha + resonance * (1.0f - alpha));
        
        // Set a maximum feedback amount to prevent instability.
        // If the feedback amount is too high, the filter will become unstable.
        feedback = std::min(feedback, 0.3f);

        // Calculate the output using a second-order recursive filter with feedback
        y = alpha * x + (1.0f - alpha) * y + feedback * (y - alpha * x);

        // Set the output
        outputs[OUTPUT]->setVoltage(y);
    }
};