#pragma once

class Connection
{
public:
    std::string source_module_uuid;
    unsigned int source_port_index;
    std::string destination_module_uuid;
    unsigned int destination_port_index;

    Connection(
        std::string src_mod_uuid, 
        unsigned int src_port_idx, 
        std::string dest_mod_uuid, 
        unsigned int dest_port_idx
    ) : 
    source_module_uuid(src_mod_uuid), 
    source_port_index(src_port_idx), 
    destination_module_uuid(dest_mod_uuid), 
    destination_port_index(dest_port_idx) 
    {}
};