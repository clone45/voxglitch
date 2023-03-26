// BaseModule.hpp

#pragma once

#include <vector>

// include input and output classes
#include "Input.h"
#include "Output.h"
#include "IModule.h"

class BaseModule : public IModule {
public:
    virtual void process(unsigned int sample_rate) override = 0;
    // virtual float getInputValue(std::shared_ptr<Input> input) const = 0;
    // virtual void setOutputValue(std::shared_ptr<Output> output, float value) = 0;

    /*
    // Getter methods for input/output names
    std::vector<std::shared_ptr<Input>> getInputs() {
        return input_ports;
    }

    std::vector<std::shared_ptr<Output>> getOutputs() {
        return output_ports;
    }

    std::shared_ptr<Input> getInputPort(int index) {
        return input_ports[index];
    }

    std::shared_ptr<Output> getOutputPort(int index) {
        return output_ports[index];
    }

   
    void addParameter(const std::string& name, float default_value) {
        parameters[name] = default_value;
    }

    float getParameter(const std::string& name) const {
        return parameters.at(name);
    }
    */
protected:

    std::unordered_map<std::string, float> parameters;
};
