#pragma once
#include <memory>
#include <vector>

class IModule;

// The class name "Port" was taken, so I decided to use "S-Port", or "Sport" for short.
class Sport
{

private:
    std::vector<Sport *> connected_inputs;
    std::vector<Sport *> connected_outputs;
    IModule *parent_module;
    float value;

public:
    // Constructor
    Sport(IModule *module)
    {
        this->parent_module = module;
        value = 0;
    }

    // Connect input
    void connectToInputPort(Sport *port)
    {
        connected_inputs.push_back(port);
    }

    void connectToOutputPort(Sport *port)
    {
        connected_outputs.push_back(port);
    }

    // Set value
    void setValue(float value)
    {
        this->value = value;
    }

    // Get value
    float getValue() const
    {
        return value;
    }

    // Get connected inputs
    std::vector<Sport *> getConnectedInputs() const
    {
        return connected_inputs;
    }

    std::vector<Sport *> getConnectedOutputs() const
    {
        return connected_outputs;
    }

    // Get the parent module
    IModule *getParentModule()
    {
        return(parent_module);
    }

    bool isConnected()
    {
        return connected_inputs.size() > 0 || connected_outputs.size() > 0;
    }
};
