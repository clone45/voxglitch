// BaseModule.hpp

#pragma once

#include <vector>

// include input and output classes
// #include "Input.h"
// #include "Output.h"
#include "IModule.h"

class BaseModule : public IModule {
public:
    virtual void process(unsigned int sample_rate) override = 0;

protected:

    std::unordered_map<std::string, float> parameters;
};
