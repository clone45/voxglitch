// IModule.h 

#pragma once

#include <vector>
#include <string>
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
    json_t* data = nullptr;
    std::string type = "";

public:
    virtual void process(unsigned int sample_rate) = 0;
    virtual std::vector<Sport *> getOutputPorts() = 0;
    virtual std::vector<Sport *> getInputPorts() = 0;
    virtual void setParameter(unsigned int param_id, float value) = 0;
    virtual Sport* getOutputPort(unsigned int port_id) = 0;
    virtual Sport* getInputPort(unsigned int port_id) = 0;
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