//
// Patch.hpp
//

#pragma once

#include "Sport.hpp"
#include "IModule.h"

class Patch
{

private:

    std::vector<IModule *> modules = {};
    IModule *terminal_output_module = nullptr;    

public:

    // Modules are assumed to be sorted in the order
    // that they need to be processed

    void setModules(std::vector<IModule *> modules)
    {
        this->modules = modules;
    }

    std::vector<IModule *>& getModules()
    {
        return this->modules;
    }

    void setTerminalOutputModule(IModule *terminal_output_module)
    {
        this->terminal_output_module = terminal_output_module;
    }

    IModule * getTerminalOutputModule()
    {
        return this->terminal_output_module;
    }

    void clear()
    {
        modules.clear();
        terminal_output_module = nullptr;
    }

};