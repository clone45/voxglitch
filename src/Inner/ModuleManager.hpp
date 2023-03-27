// ModuleManager.hpp
#pragma once

#include "Sport.hpp"
#include "IModule.h"

#include "submodules/VCOModule.hpp"
#include "submodules/OutputModule.hpp"
#include "submodules/LFOModule.hpp"

#include <map>
#include <unordered_map>
#include <memory>

struct ModuleConfig
{
    std::string name;
    std::string type;
    std::map<std::string, float> params;

    // Add this constructor to accept the 5 arguments
    ModuleConfig(
        const std::string &name, 
        const std::string &type, 
        const std::map<std::string, float> &params)
        : name(name), type(type), params(params) {}
};

class ModuleManager
{

private:
    std::map<std::string, IModule *> modules;

    // The following code creates a vector of ModuleConfig objects
    // called module_config_datas. The purpose is to store the
    // configuration data for all modules in the system.

    // std::vector<ModuleConfig> module_config_datas;

    // The input_ports and output_ports variables are used to store the input and output ports for the component.
    // The key for the unordered_map is the port name, and the value is a pointer to the port object.
    // This allows for efficient lookup of a port by name.
    // The ports are stored as pointers to allow for easy creation of new ports.
    // This is necessary because the port constructor has a reference to the component, which is not yet created
    // when the component constructor is called.
    std::unordered_map<std::string, Sport *> input_port_mappings;
    std::unordered_map<std::string, Sport *> output_port_mappings;

    // module_config_map is a map of module names to module configs
    std::unordered_map<std::string, ModuleConfig *> module_config_map;

    // Connections is a map where connections[output_port_name] = input_port_name
    std::vector<std::pair<std::string, std::string>> connections_config_forward;    

public:

    bool ready = false;

    // Constructor
    ModuleManager()
    {
        // For now, I'm going to hard-code the configuration information, but
        // eventually I'll load it from a file
        loadConfig();

        // instantiate all modules
        instantiateModules();

        // connect all modules
        if(connectModules()) // << here's where it's crashing!!
        {
            ready = true;
        }
    }

    bool isReady()
    {
        return ready;
    }

    void loadConfig()
    {

        ModuleConfig *lfo_config = createModuleConfig(
            "lfo1",                              // name
            "LFO",                               // type
            {{"frequency", 20.0}}               // params
        );

        ModuleConfig *vco_config = createModuleConfig(
            "vco1",                              // name
            "VCO",                               // type
            {{"frequency", 440.0}}               // params
        );

        ModuleConfig *outConfig = createModuleConfig(
            "output1",
            "OUTPUT",
            {}
        );

        // Remember, this is input => output
        connections_config_forward = {
            {"lfo1.OUTPUT_PORT", "vco1.FREQUENCY_INPUT_PORT"},
            {"vco1.OUTPUT_PORT", "output1.INPUT_PORT"}
        };

        // std::vector<std::string> names;

        module_config_map[lfo_config->name] = lfo_config;
        module_config_map[vco_config->name] = vco_config;
        module_config_map[outConfig->name] = outConfig;
    }

    ModuleConfig *createModuleConfig(
        const std::string &name,
        const std::string &type,
        const std::map<std::string, float> &params)
    {
        return new ModuleConfig(name, type, params);
    }

    //
    // instantiateAllModules()
    //
    // This function creates instances of the module classes and adds them to the
    // "modules" map, where the map is in the format of: modules[name] = module
    //

    void instantiateModules()
    {
        // iterate over module_configs and create instances of the module classes
        for (const auto &module_config_data : module_config_map)
        {
            ModuleConfig *config = module_config_data.second;

            std::string type = config->type;
            std::string name = config->name;

            DEBUG(("Creating module " + name + " of type " + type).c_str());

            IModule *module = nullptr;

            try
            {
                if (type == "OUTPUT")
                {
                    module = new OutputModule();
                }
                else if (type == "VCO")
                {
                    module = new VCOModule();
                }
                else if (type == "LFO")
                {
                    module = new LFOModule();
                }
                else {
                    DEBUG(("Unknown module type: " + type).c_str());
                }
            }
            catch (const std::exception &e)
            {
                std::string error = e.what();
                DEBUG(error.c_str());
                // handle the exception
            }

            if (module != nullptr)
            {
                modules[name] = module;
            }
        }
    }

    bool connectModules()
    {
        // iterate over module_configs and create instances of the module classes

        for (const auto &connection : connections_config_forward)
        {
            // Here, "connection" is a pair of strings.  
            // The first string is the fully qualified name of the output port
            // The second string is the fully qualifed name of the connected input port 
            // Each string is in the format {module_name}.{port_name}  
            // For example vco1.OUTPUT_PORT

            std::string output_connection_string = connection.first;
            std::string input_connection_string = connection.second;

            DEBUG(("Connecting ports " + output_connection_string + " to " + input_connection_string).c_str());

            // output_connection_string may look like vco1.OUTPUT_PORT
            // input_connection_string may looks like adsr1.TRIGGER_INPUT_PORT

            std::pair<std::string, std::string> output_parts = split_string(output_connection_string);
            std::pair<std::string, std::string> input_parts = split_string(input_connection_string);

            std::string output_module_name = output_parts.first;
            std::string output_port_name = output_parts.second;
            std::string input_module_name = input_parts.first;
            std::string input_port_name = input_parts.second;

            IModule *output_module;
            IModule *input_module;

            // Get the output module
            try {
                output_module = modules.at(output_module_name);
            } catch (std::out_of_range &e) {
                DEBUG(("output module not found: [" + output_module_name + "]").c_str());
                return false;
            }

            // Get the input module
            try {
                input_module = modules.at(input_module_name);
            } catch (std::out_of_range &e) {
                DEBUG(("input module not found: [" + input_module_name + "]").c_str());
                return false;
            }

            // Get the input port
            Sport *input_port = input_module->getPortByName(input_port_name);
            if (!input_port) {
                DEBUG(("Failed to find input port " + input_port_name).c_str());
                return false;
            }

            // Get the output port
            Sport *output_port = output_module->getPortByName(output_port_name);
            if (! output_port) {
                DEBUG(("Failed to find output port " + output_port_name).c_str());
                return false;
            }

            input_port->connectToOutputPort(output_port);
            output_port->connectToInputPort(input_port);
        }

        return(true);
    }


    void processPatch(IModule *module, unsigned int sample_rate)
    {

        // If the module has already been processed, return
        if (module->processing) return;
        module->processing = true;

        // Here's the process:
        // 1. Get all of the inputs of the module
        // 2. Iterate over each of the inputs.
        // 3. If there's a connection, make a recursive call to processModule with the connected module
        // 4. Once all of the inputs have been processed, call the module's process function, which will use the inputs to compute the output

        // 1. Get all of the inputs of the module
        std::vector<Sport *> input_ports = module->getInputPorts();


        // 2. Iterate over each of the inputs.
        for (auto &input_port : input_ports)
        {

            // 3. If there's a connection, make a recursive call to processModule with the connected module
            if (input_port->isConnected())
            {

                // I need to reconsider this code, which assumes that there's only one connected
                // output per input.  HOwever, this may not necessarily be the case anymore.
                std::vector<Sport *> connected_outputs = input_port->getConnectedOutputs();

                if(connected_outputs.size() > 0)
                {
                    IModule *connected_module = connected_outputs[0]->getParentModule();

                    // Heads up: this is a recursive call
                    if (!connected_module->processing)
                    {
                        processPatch(connected_module, sample_rate);
                    }
                }
            }

        }

        // 4. Once all of the inputs have been processed, call the module's process function,
        // which will read the inputs, do stuff, and set the outputs
        module->process(sample_rate);
        module->processing = false;
    }

    //
    // Call this every frame
    //
    // TODO: Handle sample rate
    //
    float process()
    {
        return compute_output();
    }

    float compute_output()
    {
        // Reset all module processed flags to false
        resetProcessingFlags();

        // The "last module" will be the output module, and the last one in the chain
        // It will be processed first, then the network will be traversed in reverse
        // to compute each module's output

        // Find the last module in the chain
        // TODO: There might be multiple output modules, so this will need to be changed??
        IModule *out_module = findOutModule();

        if(! out_module)
        {
            return(4.04);
        }

        // Compute the outputs of the system by starting with the last module,
        // then working backwards through the chain.
        processPatch(out_module, 44100);

        // Get the value at the input port of the terminal output module.
        // This is computed audio value of the entire patch.
        Sport *input_port = out_module->getPortByName("INPUT_PORT");

        return (input_port->getValue());
    }

    // Reset all module processed flags to false
    void resetProcessingFlags()
    {
        for (auto &module : modules)
        {
            module.second->processing = false;
        }
    }

    IModule * findOutModule()
    {
        IModule *out_module = nullptr;

        // Find the last module in the chain
        // Why module.second? Because module is a pair: module.first is the id
        //  and module.second is the module object

        for (auto &module : modules)
        {
            if (module.second->getOutputPorts().size() == 0)
            {
                out_module = module.second;
            }
        }

        return out_module;
    }

    std::pair<std::string, std::string> split_string(std::string input_str) {
        std::string delimiter = ".";
        size_t pos = input_str.find(delimiter);

        // Check if delimiter is found
        if (pos == std::string::npos) {
            return std::make_pair("", "");
        }

        // Split string and return pair of strings
        std::string before = input_str.substr(0, pos);
        std::string after = input_str.substr(pos + 1);
        return std::make_pair(before, after);
    }

};