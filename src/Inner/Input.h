// Input.h
#pragma once
#include <memory>
#include "Output.h"

class Output;

class Input {

private:
    std::shared_ptr<Sport> connected_output;
    float value;
    std::shared_ptr<IModule> parent;

public:
    // Constructor
    Input(std::shared_ptr<IModule> module);

    // Set value
    void setValue(float value);

    // Set connected output
    void connectToOutput(std::shared_ptr<Sport> output);

    // Get connected output
    std::shared_ptr<Sport> getConnectedOutput();

    // Get value
    float getValue();

    // Check for connection
    bool isConnected();

    // Get parent
    std::shared_ptr<IModule> getParent();
};
