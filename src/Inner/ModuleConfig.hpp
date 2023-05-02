#pragma once

#include <map>
#include <memory>
#include <string>

struct ModuleConfig
{
    std::string type;
    std::map<unsigned int, float> params;
    std::string uuid;

    ModuleConfig(std::string uuid, std::string type, std::map<unsigned int, float> params)
    {
        this->uuid = uuid;
        this->type = type;
        this->params = params;
    }
};