#pragma once
#include <vector>
#include "../BaseModule.hpp"

class Mixer8Module : public BaseModule {
private:
    std::vector<Sport*> input_ports;
    std::vector<Sport*> output_ports;

    enum INPUTS {
        INPUT_1,
        INPUT_2,
        INPUT_3,
        INPUT_4,
        INPUT_5,
        INPUT_6,
        INPUT_7,
        INPUT_8,
        NUMBER_OF_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUMBER_OF_OUTPUTS
    };

    enum PARAMS {
        NUMBER_OF_PARAMS
    };

public:
    Mixer8Module() 
    {
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) 
        {
            input_ports.push_back(new Sport(this));
        }

        for (int i = 0; i < NUMBER_OF_OUTPUTS; i++) 
        {
            output_ports.push_back(new Sport(this));
        }        
    }

    void process(unsigned int sample_rate) override 
    {
        float mix_value = 0.0f;
        for (auto input_port : input_ports) 
        {
            if (input_port->isConnected()) 
            {
                mix_value += input_port->getValue();
            }
        }
        output_port->setValue(mix_value / 8.0f);
    }

    Sport* getPortByName(std::string port_name) override 
    {
        if (port_name == "OUTPUT_PORT") return output_port;
        for (auto input_port : input_ports) 
        {
            if (port_name == input_port->getName()) return input_port;
        }
        return nullptr;
    }

    std::vector<Sport*> getOutputPorts() override 
    {
        return output_ports;
    }

    std::vector<Sport*> getInputPorts() override 
    {
        return input_ports;
    }
};
