#pragma once

#include <string>
#include <fstream>

class PatchLoader
{

public:

    json_t* load(std::string filename)
    {
        std::ifstream file(filename);
        return(json_load_file(filename.c_str(), 0, nullptr));
    }
};
