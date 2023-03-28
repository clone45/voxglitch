//
// ModuleManager.hpp
//
// TODO: 
//
// - Send through sample rate
// - Figure out how to represent PI
//

#pragma once

#include "Sport.hpp"
#include "IModule.h"

#include "submodules/PitchInputModule.hpp"
#include "submodules/GateInputModule.hpp"
#include "submodules/ParamModule.hpp"
#include "submodules/VCOModule.hpp"
#include "submodules/OutputModule.hpp"
#include "submodules/LFOModule.hpp"
#include "submodules/MoogFilterModule.hpp"
#include "submodules/LPFModule.hpp"

#include <map>
#include <unordered_map>
#include <memory>

class ModuleManager
{

private:
    std::map<std::string, IModule *> modules;

    // module_config_map is a map of module names to module configs
    std::unordered_map<std::string, ModuleConfig *> module_config_map;

    // Connections is a map where connections[output_port_name] = input_port_name
    std::vector<std::pair<std::string, std::string>> connections_config_forward;    

    // Pointers
    float *pitch_ptr;
    float *gate_ptr;
    float *p1;
    float *p2;
    float *p3;


public:

    bool ready = false;

    ModuleManager(float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3)
    {
        this->pitch_ptr = pitch_ptr;
        this->gate_ptr = gate_ptr;
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;


        // For now, I'm going to hard-code the configuration information, but
        // eventually I'll load it from a file
        // loadConfig();

        // instantiate all modules
        // instantiateModules(pitch_ptr, gate_ptr, p1, p2, p3);

        // connect all modules
        // if(connectModules()) ready = true;
    }

    bool createPatch()
    {
        // It's assumed that the module_config_map and connections_config_forward have been set

        // instantiate all modules
        instantiateModules(pitch_ptr, gate_ptr, p1, p2, p3);

        // connect all modules
        if(connectModules()) ready = true;

        return ready;
    }

    bool isReady()
    {
        return ready;
    }

    /*
    void setModuleConfigMap(std::vector<ModuleConfig*> moduleConfigs)
    {
        for (ModuleConfig* config : moduleConfigs) {
            module_config_map[config->name] = config;
        }        
    }
    */

    void setModuleConfigMap(std::vector<ModuleConfig*>& moduleConfigs) 
    {
        for (ModuleConfig* config : moduleConfigs) 
        {
            module_config_map[config->name] = config;
        }
    }

    void setConnectionConfig(std::vector<std::pair<std::string, std::string>> connections)
    {
        connections_config_forward = connections;
    }

    /*
    void loadConfig()
    {
        std::vector<ModuleConfig*> moduleConfigs = {
            createModuleConfig(
                "param1", // name
                "PARAM1", // type
                {}        // params
            ),
            createModuleConfig(
                "param2", // name
                "PARAM2", // type
                {}        // params
            ),
            createModuleConfig(
                "lfo1",
                "LFO",
                {{"frequency", 20.0}}
            ),
            createModuleConfig(
                "vco1",
                "VCO",
                {{"frequency", 440.0}}
            ),
            createModuleConfig(
                "lowpass_filter1",
                "LOWPASS_FILTER",
                {{"cutoff", 1.0}}
            ),
            createModuleConfig(
                "output1",
                "OUTPUT",
                {}
            )
        };

        // Remember, this is input => output
        connections_config_forward = {
            {"param1.OUTPUT_PORT", "lfo1.FREQUENCY_INPUT_PORT"},
            {"param2.OUTPUT_PORT", "lowpass_filter1.CUTOFF_INPUT_PORT"},
            {"lfo1.OUTPUT_PORT", "vco1.FREQUENCY_INPUT_PORT"},
            {"vco1.OUTPUT_PORT", "lowpass_filter1.INPUT_PORT"},
            {"lowpass_filter1.OUTPUT_PORT", "output1.INPUT_PORT"}
        };

        for (ModuleConfig* config : moduleConfigs) {
            module_config_map[config->name] = config;
        }
    }
    */

    /*
    ModuleConfig *createModuleConfig(
        const std::string &name,
        const std::string &type,
        const std::map<std::string, float> &params)
    {
        return new ModuleConfig(name, type, params);
    }
    */

    //
    // instantiateAllModules()
    //
    // This function creates instances of the module classes and adds them to the
    // "modules" map, where the map is in the format of: modules[name] = module
    //

    void instantiateModules(float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3)
    {
        // iterate over module_configs and create instances of the module classes
        for (const auto &module_config_data : module_config_map)
        {
            ModuleConfig *config = module_config_data.second;

            std::string type = config->type;
            std::string name = config->name;
            std::map<std::string, float> params = config->params;

            DEBUG(("Creating module " + name + " of type " + type).c_str());

            IModule *module = nullptr;

            try
            {
                if (type == "PITCH_INPUT_MODULE")
                {
                    module = new PitchInputModule(pitch_ptr);
                }
                if (type == "GATE_INPUT_MODULE")
                {
                    module = new GateInputModule(gate_ptr);
                } 
                else if (type == "OUTPUT")
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
                else if (type == "PARAM1")
                {
                    module = new ParamModule(p1);
                }
                else if (type == "PARAM2")
                {
                    module = new ParamModule(p2);
                }
                else if (type == "PARAM3")
                {
                    module = new ParamModule(p3);
                }
                else if (type == "MOOG_FILTER")
                {
                    module = new MoogFilterModule();
                }
                else if (type == "LOWPASS_FILTER")
                {
                    module = new LPFModule();
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

                // Iterate over the params and set them on the module
                for (const auto &param : params)
                {
                    std::string param_name = param.first;
                    float param_value = param.second;

                    DEBUG(("Setting param " + param_name + " to " + std::to_string(param_value)).c_str());

                    module->setParameter(param_name, param_value);
                }
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

    std::pair<std::string, std::string> split_string(std::string input_str) 
    {
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