//
// Patch.hpp
//

#pragma once

#include "Sport.hpp"
#include "IModule.h"
// #include "Connection.hpp"
#include "VoxbuilderLogger.hpp"
#include "ModuleConfig.hpp"

#include <map>
#include <unordered_map>
#include <string>

class Patch
{

private:

    // Modules are stored in a map with their uuid as the key and a pointer to the module as the value
    std::map<std::string, IModule *> modules = {};
    IModule *terminal_output_module = nullptr;    

public:

    void setModules(std::map<std::string, IModule *> modules)
    {
        this->modules = modules;
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