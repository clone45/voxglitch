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
    OutputPort * connected_output;
    bool is_connected = false;

public:

    // Constructor
    InputPort(IModule *module)
    {
        this->parent_module = module;
    }

    void connectToOutputPort(OutputPort *port)
    {
        connected_output = port;
        is_connected = true;
    }

    void setVoltage(float voltage)
    {
        this->lastVoltage = voltage;
        this->voltage = voltage;
    }

    OutputPort *getConnectedOutput() const
    {
        return connected_output;
    }

    bool isConnected()
    {
        return is_connected;
    }
};