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

    // setModules(modules_map) is called by PatchContructor.  PatchContructor has the modules 
    // stored in a map with their uuid as the key and a pointer to the module as the value.  
    // But we don't need the uuids anymore, so we just store the pointers in a vector.

    void setModules(std::map<std::string, IModule *> modules_map)
    {
        for (auto &module : modules_map)
        {
            this->modules.push_back(module.second);
        }
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