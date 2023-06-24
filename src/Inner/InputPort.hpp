#pragma once
#include <memory>
#include <vector>
#include "Sport.hpp"

class IModule;
class OutputPort;

// The class name "Port" was taken, so I decided to use "S-Port", or "Sport" for short.
class InputPort : public Sport
{

private:
    std::vector<OutputPort *> connected_outputs;

public:

    // Constructor
    InputPort(IModule *module)
    {
        this->parent_module = module;
    }

    void connectToOutputPort(OutputPort *port)
    {
        connected_outputs.push_back(port);
    }

    void setVoltage(float voltage)
    {
        this->lastVoltage = voltage;
        this->voltage = voltage;
    }

    std::vector<OutputPort *> getConnectedOutputs() const
    {
        return connected_outputs;
    }

    bool isConnected()
    {
        return connected_outputs.size() > 0;
    }
};