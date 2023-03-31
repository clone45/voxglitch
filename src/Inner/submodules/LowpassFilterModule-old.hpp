#pragma once
#include <cmath>
#include <string>
#include "../BaseModule.hpp"

class LowpassFilterModule : public BaseModule
{
    
private:
    float y = 0.0f;  // output of the filter
    float alpha = 0.1f;  // filter coefficient

public:
    Sport *input_port = new Sport(this);
    Sport *cutoff_input_port = new Sport(this);
    Sport *output_port = new Sport(this);

    LowpassFilterModule()
    {
        // Set all parameters to their default values
        setParameter("cutoff", 10.0f); // 0v to 10v
    }

    void process(unsigned int sample_rate) override
    {
        // Get the audio input
        float x = input_port->getValue();

        float cutoff_voltage = cutoff_input_port->isConnected() ? cutoff_input_port->getValue() : getParameter("cutoff");

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


        // Calculate the output using a first-order recursive filter
        y = alpha * x + (1.0f - alpha) * y;

        // Set the output
        output_port->setValue(y);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "INPUT_PORT")
        {
            return input_port;
        }
        else if (port_name == "CUTOFF_INPUT_PORT")
        {
            return cutoff_input_port;
        }
        else if (port_name == "OUTPUT_PORT")
        {
            return output_port;
        }
        else
        {
            return nullptr;
        }
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return {output_port};
    }

    std::vector<Sport *> getInputPorts() override
    {
        return {input_port, cutoff_input_port};
    }
};
