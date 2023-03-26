#pragma once

#include <cmath>
#include "../BaseModule.hpp"

class VCOModule : public BaseModule {

private:
    float phase = 0.0f;

public:
    VCOModule() 
    {
        // Add input and output ports
        addOutput("this.out");
        setParameter("frequency", 440.0f);
    }

    void process(unsigned int sample_rate) override 
    {
        // Getting the parameter by name is probably pretty slow, so we should
        // probably store the parameter in a member variable and update it
        float frequency = getParameter("frequency");
        float phase_increment = frequency / 44100.0;

        // Update phase
        phase += phase_increment;
        if (phase >= 1.0f) {
            phase -= 1.0f;
        }

        // Calculate output
        float out = sinf(phase * 2.0f * 3.14159265358979323846264338327950288);

        // Set output value (this may be a bit slow and should be optimized)
        // Here's how:
        // 1. Modify addOutput to return a pointer to the output port
        // 2. Store the output port in a member variable called something like "output_port"
        // 3. Call output_port->setValue(out) here

        Sport *port = getOutputPortByName("this.out");

        port->setValue(out * 5.0f);
    }
};