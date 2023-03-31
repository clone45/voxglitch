#pragma once
#include <string>
#include "../BaseModule.hpp"

class LinearVCAModule : public BaseModule
{
private:
    float gain = 1.0f;

public:
    Sport *input_port = new Sport(this);
    Sport *gain_port = new Sport(this);
    Sport *output_port = new Sport(this);

    LinearVCAModule()
    {
        setParameter("gain", 1.0f);
    }

    void process(unsigned int sample_rate) override
    {
        float input = input_port->getValue();

        if (gain_port->isConnected())
        {
            float gain = gain_port->getValue();
            gain = clamp(gain, 0.0f, 1.0f);
        }
        else
        {
            gain = getParameter("gain");
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
