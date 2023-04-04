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
        setParameter("gain", 10.0f);
    }

    void process(unsigned int sample_rate) override
    {
        // Get input values
        float input = input_port->getValue();
        float gain_voltage = gain_port->isConnected() ? gain_port->getValue() : getParameter("gain");
        
        // Clamp gain voltage to [0,1] range
        float gain = clamp(gain_voltage / 10.0, 0.0f, 1.0f);

        // Calculate output value
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
