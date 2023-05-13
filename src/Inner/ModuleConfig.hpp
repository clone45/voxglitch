#pragma once

#include <map>
#include <memory>
#include <string>

struct ModuleConfig
{
    std::string type;
    std::string uuid;
    json_t* defaults = nullptr;
    json_t* data = nullptr;

    // ModuleConfig(std::string uuid, std::string type, std::map<unsigned int, float> params, json_t* data = nullptr)
    ModuleConfig(std::string uuid, std::string type, json_t* defaults = nullptr, json_t* data = nullptr)
    {
        this->uuid = uuid;
        this->type = type;

        // this->params = params;

        if(defaults != nullptr) this->defaults = defaults;
        if(data != nullptr) this->data = data;
    }

    ~ModuleConfig() {
        if (defaults != nullptr) {
            json_decref(defaults);
        }
        if (data != nullptr) {
            json_decref(data);
        }
    }
};

