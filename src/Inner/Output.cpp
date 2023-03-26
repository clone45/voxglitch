// Output.cpp

#include "IModule.h"
#include "Input.h"
#include "Output.h"

Output::Output(std::shared_ptr<IModule> module)
{
    parent = module;
    value = 0;
}

void Output::setValue(float value) {
    this->value = value;
}

float Output::getValue() const {
    return value;
}

/*
std::vector<std::shared_ptr<Input>> Output::getConnectedToInputs() const {
    return connected_to_inputs;
}
*/

void Output::connectToInput(std::shared_ptr<Input> input) {
    connected_to_inputs.push_back(input);
}

std::shared_ptr<IModule> Output::getParent() {
    return parent;
}
