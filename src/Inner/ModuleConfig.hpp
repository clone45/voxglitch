#pragma once

#include <map>
#include <memory>
#include <string>

struct ModuleConfig
{
    std::string name;
    std::string type;
    std::map<std::string, float> params;

    // Add this constructor to accept the 5 arguments
    ModuleConfig(
        const std::string &name, 
        const std::string &type, 
        const std::map<std::string, float> &params)
        : name(name), type(type), params(params) {}
};