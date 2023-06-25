#pragma once

//
//
// It worth noting that Connection is only used in PatchConstructor.hpp
// and doesn't have to be fast since it's only used during patch construction.
//
//

class Connection
{
public:
    std::string source_module_uuid;
    unsigned int source_port_index;
    std::string destination_module_uuid;
    unsigned int destination_port_index;
    std::string uuid = "";

    Connection(
        std::string src_mod_uuid, 
        unsigned int src_port_idx, 
        std::string dest_mod_uuid, 
        unsigned int dest_port_idx,
        std::string uuid
    ) : 
    source_module_uuid(src_mod_uuid), 
    source_port_index(src_port_idx), 
    destination_module_uuid(dest_mod_uuid), 
    destination_port_index(dest_port_idx),
    uuid(uuid)
    {

    }
};