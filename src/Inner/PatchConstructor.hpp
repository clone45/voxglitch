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
// TODO: 
// - Send through sample rate
// - Figure out how to represent PI
// - Update findOutModule to use the Type instead of the number of outputs

#pragma once

#include "Sport.hpp"
#include "IModule.h"
#include "Patch.hpp"
#include "Connection.hpp"
#include "VoxbuilderLogger.hpp"

// Utility modules
#include "submodules/PitchInputModule.hpp"
#include "submodules/GateInputModule.hpp"
#include "submodules/OutputModule.hpp"
#include "submodules/ParamModule.hpp"

// Synth modules
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
    float *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;    
    std::vector<Connection> connections_config;

public:

    bool ready = false;

    PatchConstructor(float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, float *p8)
    {
        this->pitch_ptr = pitch_ptr;
        this->gate_ptr = gate_ptr;
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;
        this->p4 = p4;
        this->p5 = p5;
        this->p6 = p6;
        this->p7 = p7;
        this->p8 = p8;
    }

    /**
     * Creates a Patch object based on the provided JSON configuration.
     *
     * @param root The root JSON object representing the patch configuration.
     * @return A pointer to the created Patch object, or nullptr if the patch creation failed.
     *
     * The function extracts the module configurations and connection configurations from the JSON.
     * It calls the `parseModulesConfiguration` function to parse the module configurations and retrieve a map of module UUIDs to ModuleConfig pointers.
     * It calls the `parseConnectionsConfiguration` function to parse the connection configurations and retrieve a vector of Connection objects.
     * Then, it calls the `instantiateModules` function to create instances of the module classes based on the module configurations.
     * The instantiated modules are stored in a map, where the key is the module UUID and the value is a pointer to the module.
     * The function then calls the `connectModules` function to establish the connections between the modules using the connection configurations.
     * If the connection process fails, the function returns nullptr.
     * Next, the function finds the terminal output module in the module map and sets it as the terminal output module for the patch.
     * If no terminal output module is found, the function returns nullptr.
     * Finally, it creates a new Patch object, sets the terminal output module and the modules map, and returns a pointer to the created Patch object.
     * The function logs messages to the VoxbuilderLogger instance to indicate the progress and any errors encountered.
     */

    Patch *createPatch(json_t* root)
    {
        VoxbuilderLogger::getInstance().log("createPatch Initiated");

        // Get the module configurations and connection configurations from the JSON
        std::unordered_map<std::string, ModuleConfig*> module_config_map = parseModulesConfiguration(root);;
        std::vector<Connection> connections_config_vector = parseConnectionsConfiguration(root);

        // Debugging notes
        // I've test module_config_map and verified that it's good going into this function
        // It has keys as uuids and values as pointers to ModuleConfig objects.
        // The uuids are populated correctly.

        // instantiate all modules
        std::map<std::string, IModule*> modules_map = instantiateModules(module_config_map, pitch_ptr, gate_ptr, p1, p2, p3, p4, p5, p6, p7, p8);

        // Debugging notes
        // The modules_map does not have the uuid keys set properly.  
        // This is because the uuid is set in the constructor of the module, and
        // the module is created in the instantiateModules function.

        VoxbuilderLogger::getInstance().log("FOR TESTING");
        
        for (const auto& uuid_module_ptr_map : modules_map)
        {
            // log it
            VoxbuilderLogger::getInstance().log("Module uuid: " + uuid_module_ptr_map.first + ", type: " + uuid_module_ptr_map.second->uuid);
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
     * @param pitch_ptr A pointer to the pitch value.
     * @param gate_ptr A pointer to the gate value.
     * @param p1 A pointer to the parameter value for PARAM1.
     * @param p2 A pointer to the parameter value for PARAM2.
     * @param p3 A pointer to the parameter value for PARAM3.
     * @param p4 A pointer to the parameter value for PARAM4.
     * @param p5 A pointer to the parameter value for PARAM5.
     * @param p6 A pointer to the parameter value for PARAM6.
     * @param p7 A pointer to the parameter value for PARAM7.
     * @param p8 A pointer to the parameter value for PARAM8.
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

    std::map<std::string, IModule *> instantiateModules(std::unordered_map<std::string, ModuleConfig*> module_configurations, float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, float *p8)
    {
        // This is what we want to populate and return
        std::map<std::string, IModule*> modules;

        // iterate over module_configs and create instances of the module classes
        for (const auto &module_config_data : module_configurations)
        {
            ModuleConfig *config = module_config_data.second;

            std::string module_uuid = config->uuid;
            std::string type = config->type;

            json_t* data = config->data;
            json_t* defaults = config->defaults;

            IModule *module = nullptr;

            try
            {
                if (type == "AD") module = new ADModule();
                if (type == "ADDITION") module = new AdditionModule();
                if (type == "ADSR") module = new ADSRModule();
                if (type == "AMPLIFIER") module = new AmplifierModule();
                if (type == "CLOCK") module = new ClockModule();
                if (type == "CLOCK_DIVIDER") module = new ClockDividerModule();
                if (type == "DISTORTION") module = new DistortionModule();
                if (type == "DELAY") module = new DelayModule();
                if (type == "DELAY_LINE") module = new DelayLineModule();
                if (type == "DIVISION") module = new DivisionModule();
                if (type == "EXPONENTIAL_VCA") module = new ExponentialVCAModule();
                if (type == "FUZZ") module = new FuzzModule();
                if (type == "GATE_INPUT") module = new GateInputModule(gate_ptr);
                if (type == "LFO") module = new LFOModule();
                if (type == "LINEAR_VCA") module = new LinearVCAModule();
                if (type == "LOWPASS_FILTER") module = new LowpassFilterModule();
                if (type == "MIXER2") module = new Mixer2Module();
                if (type == "MIXER3") module = new Mixer3Module();
                if (type == "MIXER4") module = new Mixer4Module();
                if (type == "MIXER8") module = new Mixer8Module();
                if (type == "MOOG_LOWPASS_FILTER") module = new MoogLowpassFilterModule();
                if (type == "MORPHING_FILTER") module = new MorphingFilterModule();
                if (type == "MULTIPLICATION") module = new MultiplicationModule();
                if (type == "NOISE") module = new NoiseModule();
                if (type == "OUTPUT") module = new OutputModule();
                if (type == "OVERDRIVE") module = new OverdriveModule();
                if (type == "PARAM1") module = new ParamModule(p1);
                if (type == "PARAM2") module = new ParamModule(p2);
                if (type == "PARAM3") module = new ParamModule(p3);
                if (type == "PARAM4") module = new ParamModule(p4);
                if (type == "PARAM5") module = new ParamModule(p5);
                if (type == "PARAM6") module = new ParamModule(p6);
                if (type == "PARAM7") module = new ParamModule(p7);
                if (type == "PARAM8") module = new ParamModule(p8);
                if (type == "PITCH_INPUT") module = new PitchInputModule(pitch_ptr);
                if (type == "RAMP_OSCILLATOR") module = new RampOscillatorModule();
                if (type == "SAMPLE_AND_HOLD") module = new SampleAndHoldModule();
                if (type == "SCALE_QUANTIZER") module = new ScaleQuantizerModule();
                if (type == "SCHROEDER_REVERB") module = new SchroederReverbModule();
                if (type == "SUBTRACTION") module = new SubtractionModule();
                if (type == "SELECTOR2") module = new Selector2Module();
                if (type == "SELECTOR3") module = new Selector3Module();
                if (type == "SELECTOR4") module = new Selector4Module();
                if (type == "SELECTOR6") module = new Selector6Module();
                if (type == "SELECTOR8") module = new Selector8Module();
                if (type == "TABLE_LOOKUP") module = new TableLookupModule(data);
                if (type == "TB303_OSCILLATOR") module = new TB303OscillatorModule();
                if (type == "TB303_FILTER") module = new TB303FilterModule();
                if (type == "VCO") module = new VCOModule();
                if (type == "VOLTAGE_SEQUENCER") module = new VoltageSequencerModule(data);
                if (type == "WAVE_FOLDER") module = new WaveFolderModule();
                if (type == "WAVE_SHAPER") module = new WaveShaperModule();
                if (type == "WAVETABLE_OSCILLATOR") module = new WavetableOscillatorModule();

                if(module == nullptr) 
                {
                    VoxbuilderLogger::getInstance().log("PatchConstructor.hpp::instantiateModules() - Unknown module type: " + type);
                }
                else
                {
                    VoxbuilderLogger::getInstance().log("Created module of type " + type + " having uuid " + module_uuid);
                }
            }
            catch (const std::exception &e)
            {
                std::string error = e.what();
                VoxbuilderLogger::getInstance().log("PatchConstructor.hpp::instantiateModules()" + error);
            }

            if (module != nullptr)
            {
                // Example defaults look like:
                //
                // "defaults": {
                //    "BPM": 120
                // },

                // Iterate over the defaults and set them on the module
                if(defaults != nullptr)
                {
                    // const char *key;
                    json_t *value;
                    void *iter = json_object_iter(defaults);
                    unsigned int key_index = 0;

                    while(iter)
                    {
                        value = json_object_iter_value(iter);

                        if(json_is_number(value)) // Checks for real or integer values
                        {
                            float real_value = static_cast<float>(json_number_value(value));
                            module->setParameter(key_index, real_value);
                        }

                        iter = json_object_iter_next(defaults, iter);
                        key_index++;
                    }
                }

                module->setUuid(module_uuid);
                modules[module_uuid] = module;
            }
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
    // Note: If I update this function to find the output module by type,
    // I may be able to remove the getOutputPorts function from all modules
    // and from the IModule interface
    IModule *findOutModule(std::map<std::string, IModule*> modules_map)
    {
        IModule *out_module = nullptr;

        // Find the last module in the chain
        // Why module.second? Because module is a pair: module.first is the id
        //   and module.second is the module object

        for (auto &module : modules_map)
        {
            if (module.second->getOutputPorts().size() == 0)
            {
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

    /**
     * Parses the modules configuration from the given JSON object and returns a map of module configurations.
     *
     * @param root A pointer to the JSON object containing the modules configuration.
     * @return An unordered map of module configurations, where the keys are module UUIDs and the values are pointers to ModuleConfig objects.
     *
     * The function iterates over the modules in the JSON object and extracts the UUID, type, defaults, and data for each module.
     * It creates a new ModuleConfig object for each module and adds it to a vector.
     * Finally, it converts the vector of module configurations into an unordered map, using the UUIDs as keys, and returns the map.
     * If the JSON object is empty or does not contain any modules, an empty map is returned.
     * The function logs messages to the VoxbuilderLogger instance for important events and errors during the parsing process.
     * The logged messages include information about the number of modules found, any missing UUIDs or types, and the contents of the resulting module configuration map.
     */
    std::unordered_map<std::string, ModuleConfig*> parseModulesConfiguration(json_t* root)
    {
        std::vector<ModuleConfig*> module_config_vector;

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

            //
            //  Load "defaults"
            // 
            json_t* defaults = nullptr;
            if(json_t* defaults_obj = json_object_get(module_obj, "defaults")) 
            {
                defaults = json_deep_copy(defaults_obj);
            }

            //
            //  Load "data"
            // 
            json_t* data = nullptr;
            if(json_t* data_obj = json_object_get(module_obj, "data")) 
            {
                data = json_deep_copy(data_obj);
            }

            module_config_vector.push_back(new ModuleConfig(uuid, type, defaults, data));
        }

        std::unordered_map<std::string, ModuleConfig*> module_config_map = convertToMap(module_config_vector);

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

        return(module_config_map);
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

            json_t* src_obj = json_object_get(connection_obj, "src");
            std::string src_module_uuid = json_string_value(json_object_get(src_obj, "module_uuid"));
            unsigned int src_port_id = json_integer_value(json_object_get(src_obj, "port_id"));

            json_t* dst_obj = json_object_get(connection_obj, "dst");
            std::string dst_module_uuid = json_string_value(json_object_get(dst_obj, "module_uuid"));
            unsigned int dst_port_id = json_integer_value(json_object_get(dst_obj, "port_id"));

            connections_config.push_back(Connection(src_module_uuid, src_port_id, dst_module_uuid, dst_port_id));
        }

        return connections_config;
    }

};