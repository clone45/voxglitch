#pragma once
#include <string> 
#include <cmath>
#include "../BaseModule.hpp"

class VCOModule : public BaseModule {

private:
    float phase = 0.0f;

public:

    Sport *output_port = nullptr;

    VCOModule() 
    {
        setParameter("frequency", 440.0f);
        output_port = new Sport(this);
    }

    void process(unsigned int sample_rate) override 
    {
        // Getting the parameter by name is probably pretty slow, so we should
        // probably store the parameter in a member variable and update it
        // float frequency = getParameter("frequency");
        float frequency = 440.0;
        float phase_increment = frequency / 44100.0;

        // Update phase
        phase += phase_increment;
        if (phase >= 1.0f) {
            phase -= 1.0f;
        }

        // Calculate output
        float out = sinf(phase * 2.0f * 3.14159265358979323846264338327950288);

        // Set output value, which will also alert any connected ports
        output_port->setValue(out * 5.0f);

        // DEBUG(std::to_string(out).c_str());
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