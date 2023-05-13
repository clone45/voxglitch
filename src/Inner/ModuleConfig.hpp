#pragma once

#include <map>
#include <memory>
#include <string>

struct ModuleConfig
{
    std::string type;
    // std::map<unsigned int, float> params; // TODO: change this to "defaults"
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

    // get the default value for a parameter
    float getDefaultValue(unsigned int param_id)
    {
        if(defaults != nullptr)
        {
            json_t* param = json_object_get(defaults, std::to_string(param_id).c_str());
            if(param != nullptr)
            {
                return json_real_value(param);
            }
        }

        return 0.0;
    }

};