#pragma once

#include <map>
#include <memory>
#include <string>

struct ModuleConfig
{
    std::string type;
    std::map<unsigned int, float> params;
    std::string uuid;
    json_t* data = nullptr;

    ModuleConfig(std::string uuid, std::string type, std::map<unsigned int, float> params, json_t* data = nullptr)
    {
        this->uuid = uuid;
        this->type = type;
        this->params = params;

        if(data != nullptr) this->data = data;
    }
};