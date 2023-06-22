/*

    PatchConstructor.hpp
    █▀█ ▄▀█ ▀█▀ █▀▀ █░█   █▀▀ █▀█ █▄░█ █▀ ▀█▀ █▀█ █░█ █▀▀ ▀█▀ █▀█ █▀█
    █▀▀ █▀█ ░█░ █▄▄ █▀█   █▄▄ █▄█ █░▀█ ▄█ ░█░ █▀▄ █▄█ █▄▄ ░█░ █▄█ █▀▄
    Text created using https://fsymbols.com/generators/carty/
    
*/

//
// This class takes JSON as input and creates a Patch object.
// Patches need to be "runnable".  Eventually, I may store more than one
// patch in memory, and they'll need to be loaded in advance.
//


#pragma once

#include <windows.h> // Add this line for Windows API functions
// windows.h defines IN and OUT, which are used in my code.  For now,
// I'm undefining them here, but I should probably find a better solution.
#undef IN
#undef OUT

#include "Sport.hpp"
#include "IModule.h"
#include "Patch.hpp"
#include "Connection.hpp"
#include "VoxbuilderLogger.hpp"
#include "C:/Code/bonsaiyo/includes/VPlugin.hpp"

// Utility modules
#include "submodules/PitchInputModule.hpp"
#include "submodules/GateInputModule.hpp"
#include "submodules/OutputModule.hpp"
#include "submodules/ParamModule.hpp"

// Synth modules
/*
#include "submodules/ADSRModule.hpp"
#include "submodules/ADModule.hpp"
#include "submodules/AdditionModule.hpp"
#include "submodules/AmplifierModule.hpp"
#include "submodules/ClockDividerModule.hpp"
#include "submodules/ClockModule.hpp"
#include "submodules/DelayModule.hpp"
#include "submodules/DelayLineModule.hpp"
#include "submodules/DistortionModule.hpp"
#include "submodules/DivisionModule.hpp"
#include "submodules/ExponentialVCAModule.hpp"
#include "submodules/FuzzModule.hpp"
#include "submodules/LinearVCAModule.hpp"
#include "submodules/LFOModule.hpp"
#include "submodules/LowpassFilterModule.hpp"
#include "submodules/Mixer2Module.hpp"
#include "submodules/Mixer3Module.hpp"
#include "submodules/Mixer4Module.hpp"
#include "submodules/Mixer8Module.hpp"
#include "submodules/MoogLowpassFilterModule.hpp"
#include "submodules/MorphingFilterModule.hpp"
#include "submodules/MultiplicationModule.hpp"
#include "submodules/NoiseModule.hpp"
#include "submodules/OverdriveModule.hpp"
#include "submodules/RampOscillatorModule.hpp"
#include "submodules/SampleAndHoldModule.hpp"
#include "submodules/ScaleQuantizerModule.hpp"
#include "submodules/SchroederReverbModule.hpp"
#include "submodules/Selector2Module.hpp"
#include "submodules/Selector3Module.hpp"
#include "submodules/Selector4Module.hpp"
#include "submodules/Selector6Module.hpp"
#include "submodules/Selector8Module.hpp"
#include "submodules/SubtractionModule.hpp"
#include "submodules/TableLookupModule.hpp"
#include "submodules/TB303OscillatorModule.hpp"
#include "submodules/TB303FilterModule.hpp"
#include "submodules/VCOModule.hpp"
#include "submodules/VoltageSequencerModule.hpp"
#include "submodules/WaveFolderModule.hpp"
#include "submodules/WaveShaperModule.hpp"
#include "submodules/WavetableOscillatorModule.hpp"
*/

#include "submodules/ProxyModule.hpp"

#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>


class PatchConstructor
{

private:
    // Pointers
    // Patch *patch = nullptr;
    float *pitch_ptr;
    float *gate_ptr;
    // float *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;

    // "Adapters" are vectors of floats that will be used to pass values from the host to the plugin.
    // In other words, the host will write values to the adapter, and the plugin will read values from the adapter.
    // This is how things such as pitch and gate will be passed from the host to the plugins.
    std::vector<float *> input_adapters;
    std::vector<float *> output_adapters;

    std::vector<Connection> connections_config;

    // DLL instances that will need to be freed
    std::vector<HINSTANCE> dll_instances;

public:

    bool ready = false;

    PatchConstructor(std::vector<float *> input_adapters, std::vector<float *> output_adapters)
    {
        this->input_adapters = input_adapters;
        this->output_adapters = output_adapters;
    }

    /**
     * Creates a Patch object based on the provided JSON configuration.
     *
     * @param root The root JSON object representing the patch configuration.
     * @return A pointer to the created Patch object, or nullptr if the patch creation failed.
     *
     */

    Patch *createPatch(json_t* root)
    {
        VoxbuilderLogger::getInstance().log("createPatch Initiated");

        // Get the module configurations and connection configurations from the JSON
        std::unordered_map<std::string, ModuleConfig*> module_config_map;
        std::vector<Connection> connections_config_vector;

        std::tie(module_config_map, connections_config_vector) = parsePatchConfiguration(root, "");

        // Bridge macro connections
        bridgeMacroConnections(module_config_map, connections_config_vector);

        // instantiate all modules
        std::map<std::string, IModule*> modules_map = instantiateModules(module_config_map, this->input_adapters, this->output_adapters);

        VoxbuilderLogger::getInstance().log("Instantiated modules:");
        
        for (const auto& uuid_module_ptr_map : modules_map)
        {
            // log it
            VoxbuilderLogger::getInstance().log("Module uuid: " + uuid_module_ptr_map.first);
        }

        // connect all modules.  Note that module_map is being sent in by reference
        // and will be updated with the connections.
        if(! connectModules(modules_map, connections_config_vector)) return nullptr;

        // The "last module" will be the output module, and the last one in the chain
        // It will be processed first, then the network will be traversed in reverse
        // to compute each module's output

        // Find the last module in the chain and sets the member variable "terminal_output_module"
        IModule *terminal_output_module = findOutModule(modules_map);

        // If there's no terminal output module, then the patch is invalid
        if (terminal_output_module == nullptr) 
        {
            VoxbuilderLogger::getInstance().log("No output module found.  The patch must have an output module.");
            return nullptr;
        }
        else
        {
            VoxbuilderLogger::getInstance().log("Found terminal output module with uuid " + terminal_output_module->uuid);
        }

        // Set ready flag to true
        // (Do I need the ready flag anymore?)
        VoxbuilderLogger::getInstance().log("CreatePatch was successful.  Setting 'ready' to TRUE.");

        // Populate the patch object
        Patch *patch = new Patch();
        patch->setTerminalOutputModule(terminal_output_module);
        patch->setModules(modules_map);

        return patch;
    }

    bool isReady()
    {
        return ready;
    }

    void setReady(bool ready)
    {
        this->ready = ready;
    }


    /**
     * Instantiates module classes based on the provided module configurations and returns a map of the modules,
     * where the key is the module's uuid, and the value is a pointer to the module
     *
     * @param module_configurations An unordered map of module UUIDs to corresponding ModuleConfig pointers.
     * @param input_adapters A vector of pointers to floats that will be used to pass values from the host to the plugin.
     * @return A map of the modules, where the key is the module's UUID and the value is a pointer to the module.
     *
     * The function iterates over the module configurations and creates instances of the corresponding module classes.
     * It retrieves the UUID, type, data, and defaults for each module from the ModuleConfig object.
     * Based on the module type, it instantiates the appropriate module class and assigns it to the module pointer.
     * If an unknown module type is encountered, an error message is logged.
     * The function also sets the parameter values from the defaults on the module, if provided.
     * Finally, it sets the UUID on the module, adds it to the modules map, and returns the map of modules.
     * The function logs messages to the VoxbuilderLogger instance to indicate the creation of each module and any errors encountered.
     */

    std::map<std::string, IModule *> instantiateModules
    (
        std::unordered_map<std::string, ModuleConfig*> module_configurations, 
        std::vector<float *> input_adapters,
        std::vector<float *> output_adapters
    )
    {
        // This is what we want to populate and return
        std::map<std::string, IModule*> modules;

        // iterate over module_configs and create instances of the module classes
        for (const auto &module_config_data : module_configurations)
        {
            ModuleConfig *config = module_config_data.second;

            std::string module_uuid = config->uuid;
            std::string type = config->type;

            VoxbuilderLogger::getInstance().log("PatchConstructor.hpp::instantiateModules() - Processing module of type: " + type + " with uuid: " + module_uuid);

            json_t* data = config->data;
            json_t* defaults = config->defaults;

            // Create a string representation of the data json
            std::string json_data_string = json_dumps(data, JSON_INDENT(2));

            VPlugin* plugin = this->loadDll(type, json_data_string, input_adapters, output_adapters);

            if(plugin == nullptr) 
            {
                VoxbuilderLogger::getInstance().log("Failed to load DLL or create instance.");
            }
            else
            {
                VoxbuilderLogger::getInstance().log("Successfully loaded DLL and created instance.");
            }

            VoxbuilderLogger::getInstance().log("Fetching inputs and outputs from plugin");

            // Call the plugin's getNumInputs function
            int num_inputs = plugin->getNumInputs(); 
            int num_outputs = plugin->getNumOutputs();
           

            VoxbuilderLogger::getInstance().log("Creating new proxy module with num_inputs: " + std::to_string(num_inputs) + " and num_outputs: " + std::to_string(num_outputs));

            ProxyModule *proxy_module = new ProxyModule(num_inputs, num_outputs, data, plugin);

            proxy_module->setType(type);
            proxy_module->setUuid(module_uuid);
            modules[module_uuid] = proxy_module;
        }

        return modules;
    }

    /**
     * Connects the modules based on the provided connections configuration.
     *
     * @param modules_map A map of module UUIDs to corresponding IModule pointers.
     * @param connections_config A vector of Connection objects representing the connections between modules.
     * @return A boolean indicating whether the module connections were successfully established.
     *
     * The function iterates over the connections in the connections configuration and establishes the connections between
     * the corresponding modules. It retrieves the source and destination modules and their respective ports based on the
     * UUIDs and port indices provided in the Connection objects. It then establishes the connections between the ports.
     * If any module or port is not found in the modules_map, the function logs an error message and returns false.
     * The logged error message includes the contents of the modules_map and connections_config for debugging purposes.
     * If all connections are successfully established, the function returns true.
     */

    bool connectModules(std::map<std::string, IModule*> modules_map, std::vector<Connection> connections_config)
    {
        for (const auto& connection : connections_config)
        {
            // Connections go from "source" to "destination"
            VoxbuilderLogger::getInstance().log(
                "Connecting ports from module " 
                + connection.source_module_uuid + ", port " 
                + std::to_string(connection.source_port_index) 
                + " to module " 
                + connection.destination_module_uuid + ", port " 
                + std::to_string(connection.destination_port_index)
            );

            try 
            {
                IModule* source_module = modules_map.at(connection.source_module_uuid);
                IModule* dest_module = modules_map.at(connection.destination_module_uuid);
                Sport* source_port = source_module->getOutputPort(connection.source_port_index);
                Sport* dest_port = dest_module->getInputPort(connection.destination_port_index);

                source_port->connectToInputPort(dest_port);
                dest_port->connectToOutputPort(source_port);
            } 
            catch (const std::out_of_range& e) 
            {
                VoxbuilderLogger::getInstance().log("Trouble connecting ports! Please check module and port names.");

                // Log each module's uuid and type
                VoxbuilderLogger::getInstance().log("Contents of modules_map: ");

                for (const auto& module : modules_map)
                {
                    VoxbuilderLogger::getInstance().log("Module uuid: " + module.first + ", type: " + module.second->uuid);
                }

                // Log each connection
                VoxbuilderLogger::getInstance().log("Contents of connections_config: ");

                for (const auto& connection : connections_config)
                {
                    VoxbuilderLogger::getInstance().log("Connection source module uuid: " + connection.source_module_uuid + ", source port index: " + std::to_string(connection.source_port_index) + ", destination module uuid: " + connection.destination_module_uuid + ", destination port index: " + std::to_string(connection.destination_port_index));
                }

                return false;
            }           
        }

        return true;
    }

    //
    // When parsePatchConfiguration returned both modules and connections, if there were any
    // macros in the patch, there won't be any connections that link the inside of the macro
    // to the modules outside of the macro.  
    //
    // Here's where we are:
    // 
    //   some         Macro Module (parent)
    //   module      ┌───────────────────────────────────────┐
    //   ┌─────┐     │  MACRO_INPUT_MODULE     Other Module  │
    //   │     ├── ──►     ┌─────┐            ┌─────┐        │
    //   └─────┘     │     │     ├────────────►     │        │
    //               │     └─────┘            └─────┘        │
    //               └───────────────────────────────────────┘
    //               
    //
    //
    // And here's where we want to be:
    //
    //   some         
    //   module      
    //   ┌─────┐                           Other Module       
    //   │     ├────────────────┐           ┌─────┐          
    //   └─────┘                └───────────►     │          
    //                                      └─────┘          
    //
    //
    //
    // (Diagrams created using: https://asciiflow.com/#/)
    //
    // The modules inside the macro will have their "parent_uuid" set to the macro module's uuid.
    //

    // Here is the path forward:
    // Give each connection a uuid
    // when a connection becomes orphaned, add the uuid to a list
    // then delete all of the connections in the orphaned list

    bool bridgeMacroConnections(std::unordered_map<std::string, ModuleConfig*> &modules_config_map, std::vector<Connection> &connections_config)
    {
        VoxbuilderLogger::getInstance().log("=== Bridging Macro Connections ===");

        // TOOD: WHAT IF a macro module is connected to another macro module?
        // maybe we bridge one, then restart the process until none need to be bridged?

        std::vector<ModuleConfig *> removed_modules;
        std::vector<std::string> removed_connection_uuids;

        // Iterate through all of the modules
        for (auto &module_map : modules_config_map)
        {
            ModuleConfig* module_config = dynamic_cast<ModuleConfig*>(module_map.second);

            // Find the modules that are MACRO_INPUT_PORT or MACRO_OUTPUT_PORT
            if (module_config->getType() == "MACRO_INPUT_PORT")
            {
                ModuleConfig* MACRO_INPUT_MODULE = module_config;

                // Here's the tricky part.  We need to extract the "macro_module_input_port_index" from the data json stored
                // in the module's config.  The data json is stored in the module's config as a jsont
                int macro_module_input_port_index = getIntegerFromJson(MACRO_INPUT_MODULE->data, "macro_input_port_index");
                std::string parent_macro_module_uuid = MACRO_INPUT_MODULE->getParentUuid();
                

                //
                // I'm going to be referring to this diagram both in the comments and in the code:
                //
                //   some             Macro Module (uuid)
                //   module          ┌───────────────────────────────────────┐
                //   ┌─────┐         │  MACRO_INPUT_MODULE     Other Module  │
                //   │     ├───(i)───►       ┌─────┐            ┌─────┐      │
                //   └─────┘         │       │     │────(j)─────►     │      │
                //                   │       └─────┘            └─────┘      │
                //                   └───────────────────────────────────────┘
                //

                Connection *connection_i = this->findDstConnection(connections_config, parent_macro_module_uuid, macro_module_input_port_index);
                Connection *connection_j = this->findSrcConnection(connections_config, MACRO_INPUT_MODULE->uuid, 0);

                if (connection_i != nullptr && connection_j != nullptr)
                {
                    // Bridge the connections
                    connection_i->destination_module_uuid = connection_j->destination_module_uuid;
                    connection_i->destination_port_index = connection_j->destination_port_index;

                    VoxbuilderLogger::getInstance().log("Bridged connection (MACRO_INPUT_PORT): " + connection_i->source_module_uuid + " -> " + connection_i->destination_module_uuid);

                    // At this point, it's safe to remove the MACRO_INPUT_MODULE from the modules_config_map
                    // However, due to my memory management paranoia, let's push it onto a vector and delete it later
                    removed_modules.push_back(MACRO_INPUT_MODULE);
                    removed_modules.push_back(modules_config_map[parent_macro_module_uuid]);

                    // We can also remove the connection from MACRO_INPUT_MODULE to "other module"
                    VoxbuilderLogger::getInstance().log("Queuing bridged connection for removal: " + connection_j->uuid);
                    removed_connection_uuids.push_back(connection_j->uuid);
                }
                else
                {
                    // log an error
                    VoxbuilderLogger::getInstance().log("PatchConstructor::bridgeMacroConnections: Could not bridge ports for module: " + MACRO_INPUT_MODULE->uuid);
                }
            }

            // OK, it's time to bridge the other side:
            //
            //   We have this...
            //
            //    Macro Module (uuid)
            //   ┌─────────────────────────────────────────┐         blah
            //   │     Other_Module   MACRO_OUTPUT_MODULE  │         module
            //   │       ┌─────┐            ┌─────┐        │         ┌─────┐
            //   │       │     │─────(i)────►     │        │───(j)───►     │
            //   │       └─────┘            └─────┘        │         └─────┘
            //   └─────────────────────────────────────────┘
            //
            //    And we want this...
            //
            //    Macro Module
            //   ┌─────────────────────────────────────────┐         blah
            //   │     Other_Module                        │         module
            //   │       ┌─────┐                           │         ┌─────┐
            //   │       │     │────────(i)────────────────┼─────────►     │
            //   │       └─────┘                           │         └─────┘
            //   └─────────────────────────────────────────┘

            // 
            // The general strategy is similar to the MACRO_INPUT_PORT case above.
            // 1. Find the connection linking (i) to MACRO_OUTPUT_MODULE
            // 2. Find the connection linking (j) to "blah module"
            // 3. Update i to link from other_module to "blah module"

            if (module_config->type == "MACRO_OUTPUT_PORT")
            {
                ModuleConfig* MACRO_OUTPUT_MODULE = module_config;

                int macro_module_output_port_index = getIntegerFromJson(MACRO_OUTPUT_MODULE->data, "macro_output_port_index");
                std::string parent_macro_module_uuid = module_config->parent_uuid; // This is (uuid) in the diagram above

                // Find connection going to (i)
                Connection *connection_i = this->findDstConnection(connections_config, MACRO_OUTPUT_MODULE->uuid, 0);
                Connection *connection_j = this->findSrcConnection(connections_config, parent_macro_module_uuid, macro_module_output_port_index);

                if (connection_i != nullptr && connection_j != nullptr)
                {
                    // Bridge the connections
                    connection_i->destination_module_uuid = connection_j->destination_module_uuid;
                    connection_i->destination_port_index = connection_j->destination_port_index;

                    VoxbuilderLogger::getInstance().log("Bridged connection (MACRO_OUTPUT_PORT): " + connection_i->source_module_uuid + " -> " + connection_i->destination_module_uuid);

                    // At this point, it's safe to remove the MACRO_OUTPUT_MODULE from the modules_config_map
                    // However, due to my memory management paranoia, let's push it onto a vector and delete it later
                    removed_modules.push_back(MACRO_OUTPUT_MODULE);
                    removed_modules.push_back(modules_config_map[parent_macro_module_uuid]);

                    // We can also remove the connection from MACRO_OUTPUT_MODULE to "other module"
                    VoxbuilderLogger::getInstance().log("Queuing bridged connection for removal: " + connection_j->uuid);
                    removed_connection_uuids.push_back(connection_j->uuid);
                }
                else
                {
                    // Add a warning to the log
                    VoxbuilderLogger::getInstance().log("PatchConstructor::bridgeMacroConnections: Could not bridge ports for module: " + MACRO_OUTPUT_MODULE->uuid);
                }
            }
        }

        // Remove the "removed_modules" from modules_config_map
        for (const auto& module : removed_modules)
        {
            // Check to see if the module is in the map before trying to erase it
            if (modules_config_map.find(module->uuid) != modules_config_map.end())
            {
                VoxbuilderLogger::getInstance().log("Removing bridged module: " + module->uuid);
                modules_config_map.erase(module->uuid);
            }
        }


        //
        //
        //  TODO: Remove the "removed_connections" from connections_config
        //

        connections_config.erase(
            std::remove_if(
                connections_config.begin(), 
                connections_config.end(),
                [&](const Connection& conn) 
                {
                    bool isRemoved = std::find(
                        removed_connection_uuids.begin(), 
                        removed_connection_uuids.end(), 
                        conn.uuid) != removed_connection_uuids.end();

                    // Add logging here if a connection is being removed
                    if (isRemoved) 
                    {
                        std::string logMessage = "Bridged connection " + conn.uuid + " removed";
                        VoxbuilderLogger::getInstance().log(logMessage);
                    }

                    return isRemoved;
                }
            ), 
            connections_config.end()
        );


        // Log all of the module configs
        VoxbuilderLogger::getInstance().log("Module list:");
        for (auto& module : modules_config_map)
        {
            VoxbuilderLogger::getInstance().log("    " + module.second->uuid + " " + module.second->type);
        }

        // Log all of the connection configs
        VoxbuilderLogger::getInstance().log("Connection list:");
        for (auto& conn : connections_config)
        {
            VoxbuilderLogger::getInstance().log("  Connection UUID: " + conn.uuid);
            VoxbuilderLogger::getInstance().log("    connects modules: " + conn.source_module_uuid + " -> " + conn.destination_module_uuid);
        }

        VoxbuilderLogger::getInstance().log("PatchConstructor::bridgeMacroConnections: Finished bridging macro connections");
        VoxbuilderLogger::getInstance().log("");

        return(true);
    }

    // Pass in the source module's uuid and the source port index
    Connection *findSrcConnection(std::vector<Connection>& connections_config, std::string module_uuid, unsigned int port_index)
    {
        Connection *result = nullptr;

        for (const auto& conn : connections_config)
        {
            if (conn.source_module_uuid == module_uuid &&
                conn.source_port_index == port_index)
            {
                result = const_cast<Connection*>(&conn);
                break;
            }
        }

        return(result);
    }

    // Pass in the destination module's uuid and the destination port index
    Connection *findDstConnection(std::vector<Connection>& connections_config, std::string module_uuid, unsigned int port_index)
    {
        Connection *result = nullptr;

        for (const auto& conn : connections_config)
        {
            if (conn.destination_module_uuid == module_uuid &&
                conn.destination_port_index == port_index)
            {
                result = const_cast<Connection*>(&conn);
                break;
            }
        }

        return result;
    }



    //
    // Note: If I update this function to find the output module by type,
    // I may be able to remove the getOutputPorts function from all modules
    // and from the IModule interface
    IModule *findOutModule(std::map<std::string, IModule*> modules_map)
    {
        IModule *out_module = nullptr;

        // Find the last module in the chain
        // Why module.second? Because module is a pair: module.first is the id
        //   and module.second is the IModule object

        VoxbuilderLogger::getInstance().log("Searching for terminal output module");
        
        for (auto &module : modules_map)
        {
            /*
            int num_output_ports = module.second->getOutputPorts().size();

            VoxbuilderLogger::getInstance().log("  Testing: " + module.second->getUuid() + " having " + std::to_string(num_output_ports) + " output ports");

            if (num_output_ports == 0)
            {
                VoxbuilderLogger::getInstance().log("  Found terminal output module: " + module.second->getUuid());
                out_module = module.second;
            }
            */

            // If the module is of type "RACK_OUTPUT_ADAPTER"

            if (module.second->getType() == "RACK_OUTPUT_ADAPTER")
            {
                VoxbuilderLogger::getInstance().log("  Found terminal output module: " + module.second->getUuid());
                out_module = module.second;
            }
        }

        return(out_module);
    }

    std::unordered_map<std::string, ModuleConfig*> convertToMap(std::vector<ModuleConfig*> moduleConfigs) 
    {
        std::unordered_map<std::string, ModuleConfig*> module_config_map;

        for (ModuleConfig* config : moduleConfigs) 
        {
            module_config_map[config->uuid] = config;
        }

        return(module_config_map);
    }

  
    std::pair<std::unordered_map<std::string, ModuleConfig*>, std::vector<Connection>> parsePatchConfiguration(json_t* root, std::string parent_uuid = "")
    {
        std::vector<ModuleConfig*> module_config_vector;
        std::vector<Connection> connections_config;
        std::unordered_map<std::string, ModuleConfig*> module_config_map;

        connections_config = parseConnectionsConfiguration(root);

        json_t* modules_array = json_object_get(root, "modules");
        size_t modules_size = json_array_size(modules_array);

        for (size_t i = 0; i < modules_size; ++i)
        {
            json_t* module_obj = json_array_get(modules_array, i);

            std::string uuid = json_string_value(json_object_get(module_obj, "uuid"));
            std::string type = json_string_value(json_object_get(module_obj, "type"));

            // log if the uuid is empty
            if(uuid.empty() || uuid == "none") 
            {
                VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Module uuid not present. UUID: " + uuid);
            }

            // log if the type is empty
            if(type.empty()) 
            {
                VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Module type is empty.");
            }

            // If I didn't use json_deep_copy below, problems would occur when
            // root is freed, which could cause intermittent crashes.

            // Same code as above, but with error checking
            //
            //  Load "defaults"
            // 
            json_t* defaults = nullptr;
            json_t* defaults_obj = json_object_get(module_obj, "defaults");
            if (defaults_obj)
            {
                if (!json_is_object(defaults_obj)) 
                {
                    VoxbuilderLogger::getInstance().log("parseModulesConfiguration: defaults is not a JSON object.");
                }
                else 
                {
                    defaults = json_deep_copy(defaults_obj);
                    if (defaults == nullptr)
                    {
                        VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Failed to copy defaults.");
                    }
                }
            }
            else
            {
                VoxbuilderLogger::getInstance().log("parseModulesConfiguration: defaults field not present.");
            }

            //
            //  Load "data"
            // 
            json_t* data = nullptr;
            json_t* data_obj = json_object_get(module_obj, "data");
            if (data_obj)
            {
                if (!json_is_object(data_obj)) 
                {
                    VoxbuilderLogger::getInstance().log("parseModulesConfiguration: data is not a JSON object.");
                }
                else 
                {
                    data = json_deep_copy(data_obj);
                    if (data == nullptr)
                    {
                        VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Failed to copy data.");
                    }
                }
            }
            else
            {
                VoxbuilderLogger::getInstance().log("parseModulesConfiguration: data field not present.");
            }

            module_config_vector.push_back(new ModuleConfig(uuid, type, defaults, data, parent_uuid));

            // If the module type is "MACRO", then we need to parse the patch inside of the macro.
            // We do this by calling parsePatchConfiguration recursively.
            if(type == "MACRO") 
            {
                json_t* macro_patch = json_object_get(module_obj, "patch");

                std::pair<std::unordered_map<std::string, ModuleConfig*>, std::vector<Connection>> macro_patch_result = parsePatchConfiguration(macro_patch, uuid);

                // Add the modules in the macro patch to the module_config_map
                for (const auto& module_config : macro_patch_result.first)
                {
                    module_config_map.insert(module_config);
                }

                // Add the connections in the macro patch to the connections_config
                for (const Connection& connection : macro_patch_result.second)
                {
                    connections_config.push_back(connection);
                }
            }
        }

        // Get the results from convertToMap(module_config_vector) and append them to module_config_map
        std::unordered_map<std::string, ModuleConfig*> new_module_config_map = convertToMap(module_config_vector);

        // Append the new_module_config_map to the module_config_map
        for (const auto& module_config : new_module_config_map)
        {
            module_config_map.insert(module_config);
        }

        // Test to see if the map is empty and if so, log a message
        if(module_config_map.empty()) 
        {
            VoxbuilderLogger::getInstance().log("parseModulesConfiguration: No modules found in configuration json.");

            // Also log the contents of the json for module_config_map
            VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Contents of module_config_map: ");

            for (const auto& module_config : module_config_map)
            {
                VoxbuilderLogger::getInstance().log("Module uuid: " + module_config.first + ", type: " + module_config.second->type);
            }
        }
        else
        {
            // Log how many modules were found
            VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Found " + std::to_string(module_config_map.size()) + " modules in configuration json.");
        }

        // Here's sample output, in jSON for easy reading
        // {
        //   "7042ca37-819f-4f11-8b4c-cdfd71873824": {
        //     "type": "OUTPUT",
        //     "data": {},
        //     "defaults": {}
        //   },
        //   "c6ad45a3-0013-462e-a09d-bdb42327f5b2": {
        //     "type": "PARAM2",
        //     "data": {},
        //     "defaults": {}
        //   },
        //   "66f2169c-815a-486f-881c-395b778f8eb5": {
        //     "type": "PARAM1",
        //     "data": {},
        //     "defaults": {}
        //   },
        //   "29584a27-c30c-4bf3-b065-9658e6054d79": {
        //     "type": "WAVETABLE_OSCILLATOR",
        //     "data": {},
        //     "defaults": {}
        //   }
        // }

        // log the entire module_config_map
        VoxbuilderLogger::getInstance().log("parseModulesConfiguration: Contents of module_config_map: ");

        for (const auto& module_config : module_config_map)
        {
            VoxbuilderLogger::getInstance().log("Module uuid: " + module_config.first + ", type: " + module_config.second->type);
        }

        return std::make_pair(module_config_map, connections_config);
    }


    /**
     * Parses the connections configuration from the given JSON object and returns a vector of Connection objects.
     *
     * @param root A pointer to the JSON object containing the connections configuration.
     * @return A vector of Connection objects representing the connections between modules.
     *
     * The function iterates over the connections in the JSON object and extracts the source and destination module UUIDs
     * and port IDs for each connection. It creates a new Connection object for each connection and adds it to a vector.
     * Finally, it returns the vector of Connection objects representing the connections between modules.
     * If the JSON object is empty or does not contain any connections, an empty vector is returned.
     */

    std::vector<Connection> parseConnectionsConfiguration(json_t* root)
    {
        std::vector<Connection> connections_config;

        json_t* connections_array = json_object_get(root, "connections");
        size_t connections_size = json_array_size(connections_array);

        for (size_t i = 0; i < connections_size; ++i)
        {
            json_t* connection_obj = json_array_get(connections_array, i);
            std::string port_uuid = "";

            json_t* src_obj = json_object_get(connection_obj, "src");
            std::string src_module_uuid = json_string_value(json_object_get(src_obj, "module_uuid"));
            unsigned int src_port_id = json_integer_value(json_object_get(src_obj, "port_id"));

            json_t* dst_obj = json_object_get(connection_obj, "dst");
            std::string dst_module_uuid = json_string_value(json_object_get(dst_obj, "module_uuid"));
            unsigned int dst_port_id = json_integer_value(json_object_get(dst_obj, "port_id"));

            json_t *connection_uuid_json = json_object_get(connection_obj, "uuid");
            std::string connection_uuid = "";

            if (!connection_uuid_json) {
                VoxbuilderLogger::getInstance().log("parseConnectionsConfiguration: No port_uuid found in connection.");
            }
            if (!json_is_string(connection_uuid_json)) {
                VoxbuilderLogger::getInstance().log("parseConnectionsConfiguration: port_uuid is not a string.");
            }
            else
            {
                connection_uuid = json_string_value(connection_uuid_json);
            }

            connections_config.push_back(Connection(src_module_uuid, src_port_id, dst_module_uuid, dst_port_id, connection_uuid));
        }

        //
        // Example json output for this function
        //
        //[
        //  {
        //    "dst": {
        //      "module_uuid": "7042ca37-819f-4f11-8b4c-cdfd71873824",
        //      "port_id": 0
        //    },
        //    "src": {
        //      "module_uuid": "29584a27-c30c-4bf3-b065-9658e6054d79",
        //      "port_id": 0
        //    }
        //  },
        //  {
        //    "dst": {
        //      "module_uuid": "29584a27-c30c-4bf3-b065-9658e6054d79",
        //      "port_id": 1
        //    },
        //    "src": {
        //      "module_uuid": "c6ad45a3-0013-462e-a09d-bdb42327f5b2",
        //      "port_id": 0
        //    }
        //  },
        //  {
        //    "dst": {
        //      "module_uuid": "29584a27-c30c-4bf3-b065-9658e6054d79",
        //      "port_id": 0
        //    },
        //    "src": {
        //      "module_uuid": "66f2169c-815a-486f-881c-395b778f8eb5",
        //      "port_id": 0
        //    }
        //  }
        //]


        return connections_config;
    }

    int getIntegerFromJson(json_t* data, std::string key) 
    {
        json_t* value = json_object_get(data, key.c_str());

        if (value == NULL) {
            VoxbuilderLogger::getInstance().log("get_integer_from_json: Key not found: " + std::string(key));
            return(0);
        }

        if (!json_is_integer(value)) {
            VoxbuilderLogger::getInstance().log("get_integer_from_json: Value is not an integer: " + std::string(key));
            return(0);
        }

        return json_integer_value(value);
    }

    VPlugin* loadDll(std::string module_type, std::string json_data_string, std::vector<float *> input_adapters, std::vector<float *> output_adapters)
    {
        // Convert module_type to lowercase
        std::transform(module_type.begin(), module_type.end(), module_type.begin(),
            [](unsigned char c){ return std::tolower(c); }
        );

        VoxbuilderLogger::getInstance().log("Starting to load dll for module type: " + module_type);

        std::string dllPath = asset::plugin(pluginInstance, "res/inner/plugins/" + module_type + ".dll");

        // Check to see if the DLL exists on disk using rack::system::exists(asset::plugin(...
        if (!rack::system::exists(dllPath))
        {
            // DLL does not exist, handle the error.
            VoxbuilderLogger::getInstance().log(" | DLL does not exist on disk: \"" + dllPath + "\"");
            return nullptr;
        }

        VoxbuilderLogger::getInstance().log(" | Loading dll from path: \"" + dllPath + "\"");

        HMODULE hDll = LoadLibraryA(dllPath.c_str());

        // Store the handle to the DLL module to be able to unload it later.
        dll_instances.push_back(hDll);

        if (hDll == nullptr)
        {
            // Loading DLL failed, handle the error.
            DWORD errorCode = GetLastError();
            VoxbuilderLogger::getInstance().log(" | Loading DLL failed with error code: " + errorCode);
            return nullptr;
        }
        else
        {
            using CreatePluginFunc = VPlugin* (*)(const std::string&);

            VoxbuilderLogger::getInstance().log(" | Finding the create function");

            // Get a pointer to the create function
            CreatePluginFunc createFunc = reinterpret_cast<CreatePluginFunc>(reinterpret_cast<void*>(GetProcAddress(hDll, "create")));
            if (createFunc == nullptr)
            {
                // Getting create function failed, handle the error.
                DWORD errorCode = GetLastError();
                VoxbuilderLogger::getInstance().log(" | Getting create function failed with error code: " + errorCode);
                FreeLibrary(hDll);
                return nullptr;
            }
            else
            {
                VoxbuilderLogger::getInstance().log(" | Calling the create function");

                // Call the create function to create an instance of the module
                VPlugin* vplugin(createFunc(json_data_string));

                // Inject the adapter pointers and json data string into the module
                vplugin->setInputAdapters(input_adapters);
                vplugin->setOutputAdapters(output_adapters);
                vplugin->setData(json_data_string);

                VoxbuilderLogger::getInstance().log(" | Returning the loaded dll");

                return vplugin;
                // Note: The unique_ptr will automatically delete the created module when it goes out of scope.
                // You don't need to explicitly call delete.
            }
        }
    }

};