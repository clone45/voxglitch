#pragma once
#include <string>
#include <cmath>
#include "../BaseModule.hpp"

class MoogFilterModule : public BaseModule
{

private:
    float in1 = 0.0f, in2 = 0.0f, in3 = 0.0f, in4 = 0.0f;
    float out1 = 0.0f, out2 = 0.0f, out3 = 0.0f, out4 = 0.0f;

public:
    Sport *input_port = nullptr;
    Sport *cutoff_input_port = nullptr;
    Sport *resonance_input_port = nullptr;
    Sport *output_port = nullptr;

    MoogFilterModule()
    {
        setParameter("cutoff", 7.0f); // 0v to 10v
        setParameter("resonance", 1.0f); // 0v to 10v
        input_port = new Sport(this);
        cutoff_input_port = new Sport(this);
        resonance_input_port = new Sport(this);
        output_port = new Sport(this);
    }

    void process(unsigned int sample_rate) override
    {
        // Both getParameter("cutoff") and getParameter("resonance")
        // should return values between 0 and 10.  These values will be
        // overridden if the corresponding input port is connected.
        float cutoff_voltage = getParameter("cutoff");
        float resonance_voltage = getParameter("resonance");

        // If the cutoff input port is connected, use its value instead of the parameter
        if (cutoff_input_port->isConnected()) {
            cutoff_voltage = cutoff_input_port->getValue();
        }

        // If the resonance input port is connected, use its value instead of the parameter
        if (resonance_input_port->isConnected()) {
            resonance_voltage = resonance_input_port->getValue();
        }

        // Convert the cutoff and resonance voltages to values between 0 and 1
        float cutoff = cutoff_voltage / 10.0f;
        float resonance = resonance_voltage / 10.0f;

        // Calculate the filter coefficients
        float f = 1.0f - expf(-2.0f * 3.14159265358979323846 * cutoff);
        float fb = resonance * (1.0f - 0.15f * f * f);

        // Get the audio input
        float input = input_port->getValue();

        // Do the calculations
        float out = input - out4 * fb;
        out1 += f * (tanhf(out) - tanhf(in1));
        out2 += f * (tanhf(out1) - tanhf(in2));
        out3 += f * (tanhf(out2) - tanhf(in3));
        out4 += f * (tanhf(out3) - tanhf(in4));

        in1 = out;
        in2 = out1;
        in3 = out2;
        in4 = out3;

        output_port->setValue(out4);
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
        else if (port_name == "RESONANCE_INPUT_PORT")
        {
            return resonance_input_port;
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
        return {input_port, cutoff_input_port, resonance_input_port};
    }
};
