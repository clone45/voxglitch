// IModule.h 

#pragma once

#include <vector>
#include <unordered_map>
#include "Sport.hpp"

class Sport;

class IModule {

protected:
    std::unordered_map<std::string, float> parameters;
    std::vector<Sport *> input_ports;
    std::vector<Sport *> output_ports;

public:
    virtual void process(unsigned int sample_rate) = 0;
    virtual Sport *getPortByName(std::string port_name) = 0;
    virtual std::vector<Sport *> getOutputPorts() = 0;
    virtual std::vector<Sport *> getInputPorts() = 0;

    unsigned int id = 0;
    bool processing = false;

    // Methods for setting and getting parameters
    virtual void setParameter(const std::string& name, float value) {
        parameters[name] = value;
    }

    virtual float getParameter(const std::string& name) const {
        return parameters.at(name);
    }   
};