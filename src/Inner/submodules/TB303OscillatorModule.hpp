#pragma once

#include <string>
#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"

class TB303OscillatorModule : public BaseModule {

private:
    float phase = 0.0f;
    float last_value = 0.0f;
    float accent = 1.0f;

public:
    Sport *output_port = new Sport(this);
    Sport *frequency_input_port = new Sport(this);
    Sport *resonance_input_port = new Sport(this);

    TB303OscillatorModule()
    {
        setParameter("frequency", 440.0f);
    }

    void process(unsigned int sample_rate) override 
    {
        float frequency = getParameter("frequency");

        if (frequency_input_port->isConnected()) 
        {
            float voltage = frequency_input_port->getValue();
            frequency = 8.18f * powf(2.0f, voltage);
        }

        float resonance = 0.0f;

        if (resonance_input_port->isConnected()) 
        {
            resonance = resonance_input_port->getValue();
        }

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
        output_port->setValue(output * 5.0f);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "OUTPUT_PORT"){
            return(output_port);
        }
        else if (port_name == "FREQUENCY_INPUT_PORT"){
            return(frequency_input_port);
        }
        else if (port_name == "RESONANCE_INPUT_PORT"){
            return(resonance_input_port);
        }
        else {
           return(nullptr);
        }
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return {
            output_port
        };
    }

    std::vector<Sport *> getInputPorts() override
    {
        return {    
            frequency_input_port,
            resonance_input_port
        };
    }    
};
