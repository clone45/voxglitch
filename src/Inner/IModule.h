// IModule.h 

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "InputPort.hpp"
#include "OutputPort.hpp"
#include "Sparameter.hpp"
#include "dsp/Map.hpp"


class Sport;

class IModule {

protected:
    
    std::vector<InputPort*> inputs;
    std::vector<OutputPort*> outputs;
    std::vector<Sparameter *> params;
    json_t* data = nullptr;
    std::string type = "";

public:
    virtual void process(unsigned int sample_rate) = 0;
    virtual std::vector<InputPort *>& getInputPorts() = 0;
    virtual OutputPort* getOutputPort(unsigned int port_id) = 0;
    virtual InputPort* getInputPort(unsigned int port_id) = 0;
    virtual void setData(json_t* data) = 0;
    virtual void setUuid(std::string uuid) = 0;
    virtual std::string getUuid() = 0;
    virtual unsigned int getNumInputs() = 0;
    virtual unsigned int getNumOutputs() = 0;

    void setType(std::string type) { this->type = type; }
    std::string getType() { return this->type; }

    std::string uuid = "none";
    bool processed = false;  
};