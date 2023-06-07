#pragma once

#include <map>
#include <memory>
#include <string>
#include "VoxbuilderLogger.hpp"

struct ModuleConfig
{
    std::string type;
    std::string uuid;
    std::string parent_uuid = "";
    json_t* defaults = nullptr;
    json_t* data = nullptr;

    // ModuleConfig(std::string uuid, std::string type, std::map<unsigned int, float> params, json_t* data = nullptr)
    ModuleConfig(std::string uuid, std::string type, json_t* defaults = nullptr, json_t* data = nullptr, std::string parent_uuid = "")
    {
        this->uuid = uuid;
        this->type = type;
        this->parent_uuid = parent_uuid;
        
        // Log the uuid and type
        std::string log_msg = "ModuleConfig instantiated with: uuid: " + uuid + ", type: " + type;
        VoxbuilderLogger::getInstance().log(log_msg);

        if(defaults != nullptr) this->defaults = defaults;
        if(data != nullptr) this->data = data;
    }

    ~ModuleConfig() 
    {
        if (defaults != nullptr) {
            json_decref(defaults);
        }
        if (data != nullptr) {
            json_decref(data);
        }
    }

    std::string getType() 
    {
        return type;
    }

    std::string getUuid() 
    {
        return uuid;
    }

    std::string getParentUuid() 
    {
        return parent_uuid;
    }
};

