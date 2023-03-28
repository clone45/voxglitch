#pragma once
#include <string>
#include <cmath>
#include "../BaseModule.hpp"

class LFOModule : public BaseModule
{

private:
    float phase = 0.0f;

public:
    Sport *output_port = new Sport(this);
    Sport *frequency_input_port = new Sport(this);

    LFOModule()
    {
        setParameter("frequency", 5.0f);
    }

    void process(unsigned int sample_rate) override
    {
        float frequency = getParameter("frequency");

        if (frequency_input_port->isConnected())
        {
            frequency = frequency_input_port->getValue();
        }

        float phase_increment = frequency / sample_rate;

        // Update phase
        phase += phase_increment;
        if (phase >= 1.0f)
        {
            phase -= 1.0f;
        }

        // Calculate output
        float out = sinf(phase * 2.0f * 3.14159265358979323846264338327950288);

        // Scale output to ±5V range
        out *= 5.0f;

        // Set output value, which will also alert any connected ports
        output_port->setValue(out);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "OUTPUT_PORT")
        {
            return (output_port);
        }
        else if (port_name == "FREQUENCY_INPUT_PORT")
        {
            return (frequency_input_port);
        }
        else
        {
            return (nullptr);
        }
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return {
            output_port};
    }

    std::vector<Sport *> getInputPorts() override
    {
        return {
            frequency_input_port};
    }
};
