#pragma once

#include <map>
#include <memory>
#include <string>

struct ModuleConfig
{
    std::string name;
    std::string type;
    std::map<unsigned int, float> params;

    ModuleConfig(std::string name, std::string type, std::map<unsigned int, float> params)
    {
        this->name = name;
        this->type = type;
        this->params = params;
    }
};