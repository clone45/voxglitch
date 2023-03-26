// Input.cpp

#include "Input.h"
#include "Output.h"
#include "IModule.h"

Input::Input(std::shared_ptr<IModule> module)
{
    connected_output = nullptr,
    value = 0.0f;
    parent = module;
} 

void Input::connectToOutput(std::shared_ptr<Output> output) {
    connected_output = output;
}


// Get Connected Output
std::shared_ptr<Output> Input::getConnectedOutput() {
    return connected_output;
}

float Input::getValue() {
    if (connected_output) {
        return connected_output->getValue();
    }
    else {
        return 0.0f;
    }
}

void Input::setValue(float value) {
    this->value = value;
}

std::shared_ptr<IModule> Input::getParent() {
    return parent;
}

bool Input::isConnected() {
    return connected_output != nullptr;
}