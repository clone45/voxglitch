#pragma once

#include <map>
#include <memory>
#include <string>
#include "VoxbuilderLogger.hpp"

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

        // Log the uuid and type
        std::string log_msg = "ModuleConfig instantiated with: uuid: " + uuid + ", type: " + type;
        VoxbuilderLogger::getInstance().log(log_msg);

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

