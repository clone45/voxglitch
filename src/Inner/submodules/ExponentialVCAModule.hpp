#pragma once
#include <string>
#include <cmath>
#include "../BaseModule.hpp"

class ExponentialVCAModule : public BaseModule
{
private:
    

public:
    Sport *input_port = new Sport(this);
    Sport *gain_port = new Sport(this);
    Sport *output_port = new Sport(this);

    ExponentialVCAModule()
    {
        setParameter("gain", 10.0f);
    }

    void process(unsigned int sample_rate) override
    {
        float gain = 1.0f;
        float input = input_port->getValue();

        if (gain_port->isConnected())
        {
            float gain_voltage = gain_port->getValue();
            gain_voltage = clamp(gain_voltage, -10.0f, 10.0f);
            gain = powf(10.0f, gain_voltage / 20.0f);
        }
        else
        {
            float gain_voltage = getParameter("gain");
            gain_voltage = clamp(gain_voltage, -10.0f, 10.0f);
            gain = gain_voltage / 10.0;
        }

        float output = input * gain;

        // Set output value, which will also alert any connected ports
        output_port->setValue(output);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "INPUT_PORT")
        {
            return (input_port);
        }
        else if (port_name == "GAIN_INPUT_PORT")
        {
            return (gain_port);
        }
        else if (port_name == "OUTPUT_PORT")
        {
            return (output_port);
        }
        else
        {
            return (nullptr);
        }
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return 
        {
            output_port
        };
    }

    std::vector<Sport *> getInputPorts() override
    {
        return 
        {
            input_port,
            gain_port
        };
    }
};
