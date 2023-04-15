#pragma once

#include <map>
#include <memory>
#include <string>

struct ModuleConfig
{
    std::string type;
    std::map<unsigned int, float> params;
    unsigned int id;

    ModuleConfig(unsigned int id, std::string type, std::map<unsigned int, float> params)
    {
        this->id = id;
        this->type = type;
        this->params = params;
    }
};