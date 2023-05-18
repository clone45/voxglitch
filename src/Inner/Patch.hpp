//
// Patch.hpp
//

#pragma once

#include "Sport.hpp"
#include "IModule.h"
#include "Connection.hpp"
#include "VoxbuilderLogger.hpp"
#include "ModuleConfig.hpp"

#include <map>
#include <unordered_map>
// #include <memory>
#include <string>
// #include <utility>

class Patch
{

public:

    //
    // Modules are stored in a map with their uuid as the key and a pointer to the module as the value
    //
    std::map<std::string, IModule *> modules;

    //
    // ModuleConfig objects are stored in a map with their uuid as the key and a pointer to the ModuleConfig as the value
    //
    // I may be able to remove this
    std::unordered_map<std::string, ModuleConfig *> module_config_map;

    // Connections are stored in a vector of Connection objects
    std::vector<Connection> connections_config_forward;

    IModule *terminal_output_module = nullptr;

    void clear()
    {
        modules.clear();
        module_config_map.clear();
        connections_config_forward.clear();
        terminal_output_module = nullptr;
    }
};