//
// ModuleManager.hpp
//
// TODO: 
// - Send through sample rate
// - Figure out how to represent PI
// - Update findOutModule to use the Type instead of the number of outputs

#pragma once

#include "Sport.hpp"
#include "IModule.h"

// Utility modules
#include "submodules/PitchInputModule.hpp"
#include "submodules/GateInputModule.hpp"
#include "submodules/OutputModule.hpp"
#include "submodules/ParamModule.hpp"

// Synth modules
#include "submodules/ADSRModule.hpp"
#include "submodules/DelayModule.hpp"
#include "submodules/ExponentialVCAModule.hpp"
#include "submodules/LinearVCAModule.hpp"
#include "submodules/LFOModule.hpp"
#include "submodules/LowpassFilterModule.hpp"
#include "submodules/MorphingFilterModule.hpp"
#include "submodules/NoiseModule.hpp"
#include "submodules/SchroederReverbModule.hpp"
#include "submodules/TB303OscillatorModule.hpp"
#include "submodules/TB303FilterModule.hpp"
#include "submodules/VCOModule.hpp"
#include "submodules/WavetableOscillatorModule.hpp"

#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>

class ModuleManager
{

private:
    std::map<std::string, IModule *> modules;

    // module_config_map is a map of module names to module configs
    std::unordered_map<std::string, ModuleConfig *> module_config_map;

    // Connections is a map of string like module_name.outputs.0 : module_name.inputs.3
    std::vector<std::pair<std::string, std::string>> connections_config_forward;    

    IModule *terminal_output_module = nullptr;

    // Pointers
    float *pitch_ptr;
    float *gate_ptr;
    float *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;

public:

    bool ready = false;

    ModuleManager(float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, float *p8)
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

    /*

        █▀▀ █▀█ █▄░█ █▀ ▀█▀ █▀█ █░█ █▀▀ ▀█▀   █▀█ ▄▀█ ▀█▀ █▀▀ █░█
        █▄▄ █▄█ █░▀█ ▄█ ░█░ █▀▄ █▄█ █▄▄ ░█░   █▀▀ █▀█ ░█░ █▄▄ █▀█
        Text created using https://fsymbols.com/generators/carty/

    */
    bool createPatch()
    {
        // It's assumed that the module_config_map and connections_config_forward have been set

        // instantiate all modules
        instantiateModules(pitch_ptr, gate_ptr, p1, p2, p3, p4, p5, p6, p7, p8);

        // connect all modules
        if(connectModules()) ready = true;

        // The "last module" will be the output module, and the last one in the chain
        // It will be processed first, then the network will be traversed in reverse
        // to compute each module's output

        // Find the last module in the chain and sets the member variable "terminal_output_module"
        terminal_output_module = findOutModule();

        return ready;
    }

    bool isReady()
    {
        return ready;
    }

    void setModuleConfigMap(std::vector<ModuleConfig*>& moduleConfigs) 
    {
        for (ModuleConfig* config : moduleConfigs) 
        {
            module_config_map[config->name] = config;
        }
    }

    // At this point, the connection map holds two strings
    // Here's an example of what the map looks like:
    // { "module_name.port_name" : "module_name.port_name" },
    //
    // { "param1.OUTPUT_PORT" : "wavetable_oscillator.FREQUENCY_INPUT_PORT },
    // { "param2.OUTPUT_PORT" : "wavetable_oscillator.WAVEFORM_INPUT_PORT" },
    // etc..

    // However, this will be changing to use the port index instead of the name.
    // So, the map will look like this:
    //
    // { "module_name.outputs.port_index" : "module_name.inputs.port_index" },
    //
    // { "param1.outputs.0" : "wavetable_oscillator.inputs.0 },
    // { "param2.outputs.0" : "wavetable_oscillator.inputs.1 },
    //
    // It can be assumed that the ports on the left are outputs and the ports
    // on the right are inputs, but adding the word "outputs" and "inputs" will
    // make it more clear to people.

    void setConnectionConfig(std::vector<std::pair<std::string, std::string>> connections)
    {
        connections_config_forward = connections;
    }

    //
    // instantiateAllModules()
    //
    // This function creates instances of the module classes and adds them to the
    // "modules" map, where the map is in the format of: modules[name] = module
    //

    void instantiateModules(float *pitch_ptr, float *gate_ptr, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, float *p8)
    {
        // iterate over module_configs and create instances of the module classes
        for (const auto &module_config_data : module_config_map)
        {
            ModuleConfig *config = module_config_data.second;

            std::string type = config->type;
            std::string name = config->name;

            // TODO: Similar to ports, params will no longer have names unless
            // I create some type of configuration file that maps the names to the indexes.
            // So this might turn out being std::map<int, float> params = config->params;
            std::map<unsigned int, float> params = config->params;

            DEBUG(("Creating module " + name + " of type " + type).c_str());

            IModule *module = nullptr;

            try
            {
                if (type == "ADSR") module = new ADSRModule();
                if (type == "DELAY") module = new DelayModule();
                if (type == "EXPONENTIAL_VCA") module = new ExponentialVCAModule();
                if (type == "GATE_INPUT") module = new GateInputModule(gate_ptr);
                if (type == "LFO") module = new LFOModule();
                if (type == "LINEAR_VCA") module = new LinearVCAModule();
                if (type == "LOWPASS_FILTER") module = new LowpassFilterModule();
                if (type == "MORPHING_FILTER") module = new MorphingFilterModule();
                if (type == "NOISE_MODULE") module = new NoiseModule();
                if (type == "PARAM1") module = new ParamModule(p1);
                if (type == "PARAM2") module = new ParamModule(p2);
                if (type == "PARAM3") module = new ParamModule(p3);
                if (type == "PARAM4") module = new ParamModule(p4);
                if (type == "PARAM5") module = new ParamModule(p5);
                if (type == "PARAM6") module = new ParamModule(p6);
                if (type == "PARAM7") module = new ParamModule(p7);
                if (type == "PARAM8") module = new ParamModule(p8);
                if (type == "OUTPUT") module = new OutputModule();
                if (type == "PITCH_INPUT") module = new PitchInputModule(pitch_ptr);
                if (type == "SCHROEDER_REVERB") module = new SchroederReverbModule();
                if (type == "TB303_OSCILLATOR") module = new TB303OscillatorModule();
                if (type == "TB303_FILTER") module = new TB303FilterModule();
                if (type == "VCO") module = new VCOModule();
                if (type == "WAVETABLE_OSCILLATOR") module = new WavetableOscillatorModule();

                if(module == nullptr) 
                {
                    DEBUG("Unknown module type: ");
                    DEBUG(type.c_str());
                }
            }
            catch (const std::exception &e)
            {
                std::string error = e.what();
                DEBUG(error.c_str());
            }

            if (module != nullptr)
            {
                modules[name] = module;

                // Iterate over the params and set them on the module
                for (const auto &param : params)
                {
                    unsigned int param_index = param.first;
                    float param_value = param.second;

                    // DEBUG(("Setting param " + param_name + " to " + std::to_string(param_value)).c_str());

                    // module->params[param_index].setValue(param_value);
                    module->setParameter(param_index, param_value);
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
            // Each string is in the format {module_name}.[outputs|inputs].{port_index}  
            // For example vco1.outputs.0

            std::string output_connection_string = connection.first;
            std::string input_connection_string = connection.second;

            DEBUG(("Connecting ports " + output_connection_string + " to " + input_connection_string).c_str());

            // output_connection_string may look like vco1.outputs.0
            // input_connection_string may looks like adsr1.inputs.0

            std::pair<std::string, unsigned int> output_parts = split_connection_string(output_connection_string);
            std::pair<std::string, unsigned int> input_parts = split_connection_string(input_connection_string);

            IModule *output_module;
            IModule *input_module;
            Sport *input_port;
            Sport *output_port;

            try 
            {
                output_module = modules.at(output_parts.first);
                input_module = modules.at(input_parts.first);

                unsigned int input_port_index = input_parts.second;
                unsigned int output_port_index = output_parts.second;

                // input_port = input_module->inputs[input_port_index];
                // output_port = output_module->outputs[output_port_index];

                input_port = input_module->getInputPort(input_port_index);
                output_port = output_module->getOutputPort(output_port_index);


                input_port->connectToOutputPort(output_port);
                output_port->connectToInputPort(input_port);
            } 
            catch (std::out_of_range& e) 
            {
                DEBUG("Trouble connecting ports! Please check port and module names.");
                return false;
            }           
        }

        return(true);
    }

    //
    // Note: If I update this function to find the output module by type,
    // I may be able to remove the getOutputPorts function from all modules
    // and from the IModule interface
    IModule *findOutModule()
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

        return(out_module);
    }

    
    // Written by chatgpt
    std::pair<std::string, unsigned int> split_connection_string(std::string input_str) 
    {
        std::string delimiter = ".";
        size_t pos = 0;
        std::string module_name;
        unsigned int num = 0;

        // Extract the module name
        if ((pos = input_str.find(delimiter)) != std::string::npos) {
            module_name = input_str.substr(0, pos);
            input_str.erase(0, pos + delimiter.length());
        }

        // Extract the number at the end
        if ((pos = input_str.find(delimiter)) != std::string::npos) {
            input_str.erase(0, pos + delimiter.length());
        }
        num = std::stoi(input_str);

        return std::make_pair(module_name, num);
    } 

    /*


        █▀█ █░█ █▄░█   █▀█ ▄▀█ ▀█▀ █▀▀ █░█
        █▀▄ █▄█ █░▀█   █▀▀ █▀█ ░█░ █▄▄ █▀█
        Text created using https://fsymbols.com/generators/carty/

    */

    // This runs at sample rate
    void processModule(IModule *module, unsigned int sample_rate)
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
                        processModule(connected_module, sample_rate);
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
    // process()
    //
    // Starts processing a patch to get the output.  The output of the entire
    // patch is returned as a float.
    //
    // This is called at sample rate.
    //
    // TODO: Handle passing in sample rate
    //


    float process(unsigned int sample_rate)
    {
        // Reset all module processed flags to false
        resetProcessingFlags();

        if(! terminal_output_module)
        {
            return(4.04);
        }

        // Compute the outputs of the system by starting with the last module,
        // then working backwards through the chain.
        processModule(terminal_output_module, sample_rate);

        // This might be a litte confusing, so let me explain it a bit.
        // The terminal output module is the last module in the patch. It has
        // an input port called INPUT_PORT.  The value at INPUT_PORT is basically
        // the value that the entire patch outputs.  So here, we're getting the
        // value at INPUT_PORT and returning it.  The function processModule()
        // will have computed the value at INPUT_PORT by processing the entire
        // patch, so we're just returning that value.

        Sport *input_port = terminal_output_module->getInputPort(0);

        return (input_port->getVoltage());
    }

    // Reset all module processed flags to false
    void resetProcessingFlags()
    {
        for (auto &module : modules)
        {
            module.second->processing = false;
        }
    }
};