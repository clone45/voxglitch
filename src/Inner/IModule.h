// IModule.h 

#pragma once

#include <vector>
#include <unordered_map>
#include "Sport.hpp"
#include "Sparameter.hpp"
#include "dsp/Map.hpp"

class Sport;

class IModule {

protected:
    
    std::vector<Sport*> inputs;
    std::vector<Sport*> outputs;
    std::vector<Sparameter *> params;

public:
    virtual void process(unsigned int sample_rate) = 0;
    virtual std::vector<Sport *> getOutputPorts() = 0;
    virtual std::vector<Sport *> getInputPorts() = 0;

    unsigned int id = 0;
    bool processing = false;  
};