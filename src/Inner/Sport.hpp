#pragma once
#include <memory>
#include <vector>

class IModule;

// The class name "Port" was taken, so I decided to use "S-Port", or "Sport" for short.
class Sport
{

public:
    IModule *parent_module;
    float lastVoltage = 0.0;



    // I'm making this public so that ProxyModule doesn't have to call
    // getVoltage() multiple times.  Instead, it will access it directly.
    float voltage = 0.0;

    float getLastVoltage()
    {
        return lastVoltage;
    }

    // Get value
    float getVoltage() const
    {
        return clamp(voltage, -50.0f, 50.0f);
    }

    // Get the parent module
    IModule *getParentModule()
    {
        return(parent_module);
    }
};