#pragma once

#include <string>
#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"

class TB303FilterModule : public BaseModule {

private:
    float buf0 = 0.0f;
    float buf1 = 0.0f;
    float buf2 = 0.0f;
    float buf3 = 0.0f;

public:
    Sport *output_port = new Sport(this);
    Sport *input_port = new Sport(this);
    Sport *cutoff_input_port = new Sport(this);
    Sport *resonance_input_port = new Sport(this);

    TB303FilterModule()
    {
        setParameter("cutoff", 0.5f);
        setParameter("resonance", 0.5f);
    }

    void process(unsigned int sample_rate) override 
    {
        float input = input_port->isConnected() ? input_port->getValue() : 0.0f;
        float cutoff = cutoff_input_port->isConnected() ? (cutoff_input_port->getValue() / 5.0) : getParameter("cutoff");
        float resonance = resonance_input_port->isConnected() ? (resonance_input_port->getValue() / 1.0) : getParameter("resonance");

        float fc = 110.0f * powf(2.0f, 3.0f * cutoff);
        float g = tanf(M_PI * fc / sample_rate);
        float r = resonance * (1.2f + 0.5f * g / (g + 1.0f));

        float input_t = input - r * buf3;

        // Diode ladder filter design (four one-pole low-pass filters in series)
        buf0 += g * (tanhf(input_t) - tanhf(buf0));
        buf1 += g * (tanhf(buf0) - tanhf(buf1));
        buf2 += g * (tanhf(buf1) - tanhf(buf2));
        buf3 += g * (tanhf(buf2) - tanhf(buf3));

        output_port->setValue(buf3 * 5.0f);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "OUTPUT_PORT"){
            return(output_port);
        }
        else if (port_name == "INPUT_PORT"){
            return(input_port);
        }
        else if (port_name == "CUTOFF_INPUT_PORT"){
            return(cutoff_input_port);
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
            input_port,
            cutoff_input_port,
            resonance_input_port
        };
    }    
};