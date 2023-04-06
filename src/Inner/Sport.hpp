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
    float voltage = 0.0;

public:
    // Constructor
    Sport(IModule *module)
    {
        this->parent_module = module;
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

    // Set voltage
    // This should really only be called the the Sport type is outout
    void setVoltage(float voltage)
    {
        this->voltage = voltage;

        // Iterate over connected_inputs and set the value of the inputs
        // I might want to rethink this and register observers
        for (auto &input_port : connected_inputs)
        {
            input_port->setVoltage(voltage);
        }
    }

    // Get value
    float getVoltage() const
    {
        return voltage;
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
