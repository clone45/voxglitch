#include "Sport.h"

Sport::Sport(std::shared_ptr<IModule> module)
{
    this->parent_module = module;
    value = 0;
}

void Sport::connectToInputPort(std::shared_ptr<Sport> Sport)
{
    connected_inputs.push_back(Sport);
}

void Sport::connectToOutputPort(std::shared_ptr<Sport> Sport)
{
    connected_outputs.push_back(Sport);
}

void Sport::setValue(float value)
{
    this->value = value;
}

float Sport::getValue() const
{
    return value;
}

std::vector<std::shared_ptr<Sport>> Sport::getConnectedInputs() const
{
    return connected_inputs;
}

std::vector<std::shared_ptr<Sport>> Sport::getConnectedOutputs() const
{
    return connected_outputs;
}

bool Sport::isConnected()
{
    return connected_inputs.size() > 0 || connected_outputs.size() > 0;
}

std::shared_ptr<IModule> Sport::getParentModule()
{
    return(parent_module);
}

/*
std::shared_ptr<IModule> Sport::getModule()
{
    return parent;
}
*/