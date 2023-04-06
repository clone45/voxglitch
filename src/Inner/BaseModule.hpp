// BaseModule.hpp

#pragma once

#include <vector>

#include "IModule.h"

class BaseModule : public IModule {

public:
    
    virtual void process(unsigned int sample_rate) override = 0;

    BaseModule()
    { 
    }

    std::vector<Sport*> getOutputPorts() override 
    {
        return outputs;
    }

    std::vector<Sport*> getInputPorts() override 
    {
        return inputs;
    }

    void config(unsigned int NUM_PARAMS, unsigned int NUM_INPUTS, unsigned int NUM_OUTPUTS)
    {
        for (int i = 0; i < NUM_INPUTS; i++) 
        {
            inputs.push_back(new Sport(this));
        }

        for (int i = 0; i < NUM_OUTPUTS; i++) 
        {
            outputs.push_back(new Sport(this));
        }

        for (int i = 0; i < NUM_PARAMS; i++) 
        {
            params.push_back(new Sparameter(this));
        }
    }
};