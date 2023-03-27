#pragma once
#include <string> 
#include <cmath>
#include "../BaseModule.hpp"

class GateInputModule : public BaseModule {

private:

public:

    Sport *output_port = nullptr;
    float *gate_value_ptr = nullptr;

    GateInputModule(float *gate_ptr) 
    {
        this->gate_value_ptr = gate_ptr;
        output_port = new Sport(this);
    }

    void process(unsigned int sample_rate) override 
    {
        output_port->setValue(*gate_value_ptr);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "OUTPUT_PORT"){
            return(output_port);
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
        };
    }    
};