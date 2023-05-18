
#pragma once

//
// 
// █▀█ ▄▀█ ▀█▀ █▀▀ █░█   █▀█ █░█ █▄░█ █▄░█ █▀▀ █▀█
// █▀▀ █▀█ ░█░ █▄▄ █▀█   █▀▄ █▄█ █░▀█ █░▀█ ██▄ █▀▄
// Text created using https://fsymbols.com/generators/carty/
//

#include "Sport.hpp"
#include "IModule.h"
#include "VoxbuilderLogger.hpp"
#include <stack>

class PatchRunner
{

private:

    // A stack of pointers to modules that have been processed
    std::stack<IModule *> processed_modules;

public:

    // This runs at sample rate
    void processModule(IModule *module, unsigned int sample_rate)
    {
        if (module->processing) return;
        module->processing = true;

        // Push the module onto the stack of processed modules.  When the 
        // processing of the patch is complete, we'll pop all the modules
        // off the stack and reset their processing flags to false.

        processed_modules.push(module);

        std::vector<Sport *> input_ports = module->getInputPorts();

        for (auto &input_port : input_ports)
        {
            if (input_port->isConnected())
            {
                std::vector<Sport *> connected_outputs = input_port->getConnectedOutputs();

                if(connected_outputs.size() > 0)
                {
                    IModule *connected_module = connected_outputs[0]->getParentModule();

                    // If the connected module is not currently being processed,
                    // process it.  Otherwise, use the output from the last timestep.
                    if (!connected_module->processing)
                    {
                        processModule(connected_module, sample_rate);
                    }
                    else
                    {
                        input_port->setVoltage(connected_outputs[0]->getLastVoltage());
                    }
                }
            }
        }

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
    // TODO: Handle passing in sample rate
    //
    float process(unsigned int sample_rate, IModule *terminal_output_module)
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
    // rewrite to be recursive?  Or push traversed modules onto a stack?
    void resetProcessingFlags()
    {
        while(! processed_modules.empty())
        {
            IModule *module = processed_modules.top();
            module->processing = false;
            processed_modules.pop();
        }
    }
};