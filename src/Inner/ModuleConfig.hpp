#pragma once

#include <map>
#include <memory>
#include <string>

struct ModuleConfig
{
    std::string name;
    std::string type;
    std::vector<float> params;

   ModuleConfig(std::string name, std::string type, std::vector<float> params)
   {
       this->name = name;
       this->type = type;
       this->params = params;
    }

};