#pragma once
#include <memory>
#include <vector>
#include "Sport.hpp"

class IModule;
class InputPort;

// The class name "Port" was taken, so I decided to use "S-Port", or "Sport" for short.
class OutputPort : public Sport
{

private:
    std::vector<InputPort *> connected_inputs;

public:

    // Constructor
    OutputPort(IModule *module)
    {
        this->parent_module = module;
    }

    void connectToInputPort(InputPort *port)
    {
        connected_inputs.push_back(port);
    }

    void setVoltage(float voltage)
    {
        this->lastVoltage = voltage;
        this->voltage = voltage;

        // Iterate over connected_inputs and set the value of the inputs
        for (auto &input_port : connected_inputs)
        {
            input_port->setVoltage(voltage);
        }
    }

    // Get connected inputs
    std::vector<InputPort *> getConnectedInputs() const
    {
        return connected_inputs;
    }

    bool isConnected()
    {
        return connected_inputs.size() > 0;
    }
};