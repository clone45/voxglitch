#pragma once
#include "../BaseModule.hpp"
#include "../dsp/AudioDelay.hpp"

class DelayModule : public BaseModule {

private:
    AudioDelay delay;

public:

    Sport *input_port = new Sport(this);
    Sport *output_port = new Sport(this);
    Sport *delay_time_input_port = new Sport(this);
    Sport *feedback_input_port = new Sport(this);
    Sport *mix_input_port = new Sport(this);

    DelayModule() 
    {
        setParameter("delay_time", 5.0f);
        setParameter("feedback", 9.0f);
        setParameter("mix", 9.0f);
    }

    void process(unsigned int sample_rate) override 
    {
        // Get input value
        float input = input_port->getValue();

        // Get parameters
        float delay_time_voltage = delay_time_input_port->isConnected() ? delay_time_input_port->getValue() : getParameter("delay_time");
        float feedback_voltage = feedback_input_port->isConnected() ? feedback_input_port->getValue() : getParameter("feedback");
        float mix_voltage = mix_input_port->isConnected() ? mix_input_port->getValue() : getParameter("mix");

        // Normalize voltages to values 0.0 to 1.0
        float delay_time = delay_time_voltage / 10.0f;
        float feedback = feedback_voltage / 10.0f;
        float mix = mix_voltage / 10.0f;

        // Clamp values to [0,1] range
        delay_time = clamp(delay_time, 0.0f, 1.0f);
        feedback = clamp(feedback, 0.0f, 1.0f);
        mix = clamp(mix, 0.0f, 1.0f);

        float output = delay.process(input);

        // Set output value, which will also alert any connected ports
        output_port->setValue(output);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "INPUT_PORT")
        {
            return (input_port);
        }
        else if (port_name == "OUTPUT_PORT")
        {
            return (output_port);
        }
        else if (port_name == "DELAY_TIME_INPUT_PORT")
        {
            return (delay_time_input_port);
        }
        else if (port_name == "FEEDBACK_INPUT_PORT")
        {
            return (feedback_input_port);
        }
        else if (port_name == "MIX_INPUT_PORT")
        {
            return (mix_input_port);
        }
        else
        {
            return (nullptr);
        }
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return (std::vector<Sport *> {output_port});
    }

    std::vector<Sport *> getInputPorts() override
    {
        return (std::vector<Sport *> {
            input_port, 
            delay_time_input_port, 
            feedback_input_port, 
            mix_input_port
        });
    }
};