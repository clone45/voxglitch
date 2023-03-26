// ModuleManager.hpp
#pragma once

#include "Sport.hpp"
#include "IModule.h"

#include "submodules/VCOModule.hpp"
#include "submodules/OutputModule.hpp"

#include <map>
#include <unordered_map>
#include <memory>

struct ModuleConfig
{
    std::string name;
    std::string type;
    std::map<std::string, float> params;
    std::map<std::string, std::string> inputs;
    std::map<std::string, std::string> outputs;

    // Add this constructor to accept the 5 arguments
    ModuleConfig(const std::string &name, const std::string &type, const std::map<std::string, float> &params, const std::map<std::string, std::string> &inputs, const std::map<std::string, std::string> &outputs)
        : name(name), type(type), params(params), inputs(inputs), outputs(outputs) {}
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
    std::unordered_map<std::string, Sport *> input_ports;
    std::unordered_map<std::string, Sport *> output_ports;

    // module_config_map is a map of module names to module configs
    std::unordered_map<std::string, ModuleConfig *> module_config_map;

public:
    // Constructor
    ModuleManager()
    {
        // For now, I'm going to hard-code the configuration information, but
        // eventually I'll load it from a file
        loadConfig();

        // instantiate all modules
        instantiateModules();

        // index all ports
        indexPorts();

        // connect all modules
        // connectModules();
    }

    void loadConfig()
    {

       ModuleConfig *vco_config = createModuleConfig(
            "vco1",                               // name
            "VCO",                                // type
            {{"frequency", 440.0}},               // params
            std::map<std::string, std::string>(), // inputs
            {{"this.out", "out.in"}}              // outputs
        );

        ModuleConfig *outConfig = createModuleConfig(
            "out",
            "OUTPUT",
            {},                           // params
            {{"vco1.out", "this.in"}},    // inputs
            {{"this.out", "module.out1"}} // outputs
        );

        module_config_map.insert({vco_config->name, vco_config});
        module_config_map.insert({outConfig->name, outConfig});
    }

    ModuleConfig *createModuleConfig(
        const std::string &name,
        const std::string &type,
        const std::map<std::string, float> &params,
        const std::map<std::string, std::string> &inputs,
        const std::map<std::string, std::string> &outputs)
    {
        return new ModuleConfig(name, type, params, inputs, outputs);
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
            }
            catch (const std::exception &e)
            {
                std::string error = e.what();
                DEBUG(error.c_str());
                // handle the exception
            }

            // add input ports to the module
            for (const auto &input_port_data : config->inputs)
            {
                std::string input_port_name = input_port_data.first;
                module->addInput(input_port_name);
            }

            // add output ports to the module
            for (const auto &output_port_data : config->outputs)
            {
                std::string output_port_name = output_port_data.first;
                module->addOutput(output_port_name);
            }

            if (module != nullptr)
            {
                modules[name] = module;
            }
        }

        // This function writes debugging information to the log file.  It takes a char *.
        DEBUG("Modules Instantiated");

        // Show modules
        for (const auto& module : modules)
        {
            DEBUG((module.first + ": " + typeid(*module.second).name()).c_str());
        }
    }

    void indexPorts()
    {
        // When created, modules add their ports and maintain a map of them
        // with the format: input_ports[name] = port

        // This function should:
        // 1. Iterate over the "modules" map
        // 2. For each module, iterate over the module's input ports
        // 3. Computer the full port path string, like vco1.frequency_input
        // 4. Add the input port to the ModuleManager's input_ports map
        // 5. Do the same for output ports

        // 1. iterate over modules map
        for (std::pair<const std::string, IModule *> module : modules)
        {
            std::string module_name = module.first;
            IModule *module_object = module.second;

            // 2. Next: Iterate over the module_object's input ports map

            for (const auto &input_port : module_object->getInputPorts())
            {
                // 3. input_port is a pair in the form {{ string : pointer_to_port_structure }}
                // Here, we pull the parts out of the pair for clarity:
                std::string input_port_name = input_port.first;
                Sport *input_port_object = input_port.second;

                std::string full_path_string = module_name + "." + input_port_name;

                // 4. Add the input port to the ModuleManager's input_ports map
                this->input_ports[full_path_string] = input_port_object;
            }

            // 5. Now do the same for output ports

            for (const auto &output_port : module_object->getOutputPorts())
            {
                // Output_port is a pair in the form {{ string : pointer_to_port_structure }}
                // Here, we pull the parts out of the pair for clarity:
                std::string output_port_name = output_port.first;
                Sport *output_port_object = output_port.second;

                std::string full_path_string = module_name + "." + output_port_name;

                // Add the output port to the ModuleManager's output_ports map
                this->output_ports[full_path_string] = output_port_object;
            }
        }

        // Let's debug really quickly
        // Output the input ports to the log file
        DEBUG("input ports: ");
        for (const auto& port : input_ports)
        {
            std::string port_name = port.first;
            Sport* port_object = port.second;

            std::string message = " " + port_name;
            DEBUG(message.c_str());
        }

        // Output the output ports to the log file
        DEBUG("outputs ports: ");
        for (const auto& port : output_ports)
        {
            std::string port_name = port.first;
            Sport* port_object = port.second;

            std::string message = " " + port_name;
            DEBUG(message.c_str());
        }

        /*
[1.199 debug src/Inner/ModuleManager.hpp:228 indexPorts] Input port out.this.in of type 5Sport indexed
[1.199 debug src/Inner/ModuleManager.hpp:228 indexPorts] Input port out.vco1.out of type 5Sport indexed
[1.199 debug src/Inner/ModuleManager.hpp:238 indexPorts] Output port vco1.this.out of type 5Sport indexed
[1.199 debug src/Inner/ModuleManager.hpp:238 indexPorts] Output port out.this.out of type 5Sport indexed        
        
        */

    }

    void connectModules()
    {
        // iterate over module_configs and create instances of the module classes

        for (const auto &module_config_data : module_config_map)
        {
            ModuleConfig *config = module_config_data.second;

            std::string type = config->type;
            std::string name = config->name;

            // get the module from the modules map
            IModule *module = modules[name];

            // Test if the module is valid
            if (module != nullptr)
            {
                // iterate over the module's input ports
                for (const auto &port_pair : config->inputs)
                {
                    const auto &src_port_name = port_pair.first;
                    const auto &dst_port_name = port_pair.second;

                    // get the input port from the input_ports map
                    Sport *input_port = input_ports[dst_port_name];   // on of the input ports of this module
                    Sport *output_port = output_ports[src_port_name]; // output port of remote module (not this)

                    // connect the input port to the output port
                    input_port->connectToOutputPort(output_port);
                    output_port->connectToInputPort(input_port);
                }
            }
            else
            {
                // handle error case: module not found
            }
        }
    }

    void processPatch(IModule *module, float sample_rate)
    {
        // If the module has already been processed, return
        if (module->processing)
            return;
        module->processing = true;

        // Here's the process:
        // 1. Get all of the inputs of the module
        // 2. Iterate over each of the inputs.
        // 3. If there's a connection, make a recursive call to processModule with the connected module
        // 4. Once all of the inputs have been processed, call the module's process function, which will use the inputs to compute the output

        // 1. Get all of the inputs of the module
        std::unordered_map<std::string, Sport *> inputs = module->getInputPorts();

        // 2. Iterate over each of the inputs.
        for (auto &input : inputs)
        {
            std::string input_name = input.first;
            Sport *input_port = input.second;

            // 3. If there's a connection, make a recursive call to processModule with the connected module
            if (input_port->isConnected())
            {

                // I need to reconsider this code, which assumes that there's only one connected
                // output per input.  HOwever, this may not necessarily be the case anymore.
                std::vector<Sport *> connected_outputs = input_port->getConnectedOutputs();
                IModule *connected_module = connected_outputs[0]->getParentModule();

                // Heads up: this is a recursive call
                if (!connected_module->processing)
                {
                    processPatch(connected_module, sample_rate);
                }
            }
        }

        // 4. Once all of the inputs have been processed, call the module's process function,
        // which will read the inputs, do stuff, and set the outputs
        module->process(sample_rate);
        module->processing = false;
    }

    float run(float sample_rate)
    {
        // Reset all module processed flags to false
        resetProcessingFlags();

        // The "last module" will be the output module, and the last one in the chain
        // It will be processed first, then the network will be traversed in reverse
        // to compute each module's output

        // Find the last module in the chain
        // TODO: There might be multiple output modules, so this will need to be changed
        IModule *out_module = findOutModule();

        // Compute the outputs of the system by starting with the last module,
        // then working backwards through the chain.
        processPatch(out_module, sample_rate);

        // Get the values of the last module
        Sport *output_port = out_module->getOutputPortByName("this.out");

        return (output_port->getValue());
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
};