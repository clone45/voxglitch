// Output.h

#pragma once
#include <vector>
#include "IModule.h"
#include "Input.h"

// Forward declaration of Input class
class Input;

class Output 
{

private:
    std::vector<std::shared_ptr<Sport>> connected_to_inputs;
    float value;
    std::shared_ptr<IModule> parent;

public:
    // Constructor
    Output(std::shared_ptr<IModule> module);

    // Connect input
    void connectToInput(std::shared_ptr<Sport> input);

    // Set value
    void setValue(float value);

    // Get value
    float getValue() const;

    // Get connected inputs
    std::vector<std::shared_ptr<Sport>> getConnectedToInputs() const;

    // Get parent
    std::shared_ptr<IModule> getParent();

};
