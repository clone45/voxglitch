// BaseModule.hpp

#pragma once

#include <vector>

#include "IModule.h"

class BaseModule : public IModule {

public:
    
    virtual void process(unsigned int sample_rate) override = 0;
    unsigned int number_of_inputs = 0;
    unsigned int number_of_outputs = 0;

    BaseModule()
    { 
    }

    // Currently there's no need for a getOutputPorts function
    std::vector<InputPort*>& getInputPorts() override 
    {
        return inputs;
    }

    void config(unsigned int NUM_PARAMS, unsigned int NUM_INPUTS, unsigned int NUM_OUTPUTS)
    {
        for (unsigned int i = 0; i < NUM_INPUTS; i++) 
        {
            inputs.push_back(new InputPort(this));
        }

        for (unsigned int i = 0; i < NUM_OUTPUTS; i++) 
        {
            outputs.push_back(new OutputPort(this));
        }

        number_of_inputs = NUM_INPUTS;
        number_of_outputs = NUM_OUTPUTS;
    }

    void setData(json_t* data) override
    {
        this->data = data;
    }

    void setUuid(std::string uuid) override
    {
        this->uuid = uuid;
    }

    std::string getUuid() override
    {
        return this->uuid;
    }

    OutputPort* getOutputPort(unsigned int port_id) override
    {
        return outputs[port_id];
    }

    InputPort* getInputPort(unsigned int port_id) override
    {
        return inputs[port_id];
    }

    unsigned int getNumInputs() override
    {
        return number_of_inputs;
    }

    unsigned int getNumOutputs() override
    {
        return number_of_outputs;
    }
};