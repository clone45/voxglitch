#pragma once
#include <memory>
#include <vector>

class IModule;

// The class name "Port" was taken, so I decided to use "S-Port", or "Sport" for short.
class Sport 
{

private:
    std::vector<std::shared_ptr<Sport>> connected_inputs;
    std::vector<std::shared_ptr<Sport>> connected_outputs;
    std::shared_ptr<IModule> parent_module;
    float value;

public:
    // Constructor
    Sport(std::shared_ptr<IModule> module);

    // Connect input
    void connectToInputPort(std::shared_ptr<Sport> port);
    void connectToOutputPort(std::shared_ptr<Sport> port);

    // Set value
    void setValue(float value);

    // Get value
    float getValue() const;

    // Get connected inputs
    std::vector<std::shared_ptr<Sport>> getConnectedInputs() const;
    std::vector<std::shared_ptr<Sport>> getConnectedOutputs() const;

    // Get the parent module
    std::shared_ptr<IModule> getParentModule();

    bool isConnected();

    
};
