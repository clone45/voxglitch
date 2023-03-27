#pragma once
#include <string> 
#include <cmath>
#include "../BaseModule.hpp"

class PitchInputModule : public BaseModule {

private:

public:

    Sport *output_port = nullptr;
    float *pitch_value_ptr = nullptr;

    PitchInputModule(float *pitch_ptr) 
    {
        this->pitch_value_ptr = pitch_ptr;
        output_port = new Sport(this);
    }

    void process(unsigned int sample_rate) override 
    {
        output_port->setValue(*pitch_value_ptr);
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