// OutModule.hpp

#pragma once

#include "../BaseModule.hpp"

class OutputModule : public BaseModule {
public:

    Sport *input_port = nullptr;
    float value = 0.0;

    OutputModule() {
        input_port = new Sport(this);
    }

    void process(unsigned int sample_rate) override {
        value = input_port->getValue();
        // DEBUG(std::to_string(value).c_str());
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "INPUT_PORT"){
            return(input_port);
        }
        else {
           return(nullptr);
        }
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return {
        };
    }

    std::vector<Sport *> getInputPorts() override
    {
        return {
            input_port
        };
    }      
};