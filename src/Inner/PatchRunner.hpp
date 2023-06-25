
#pragma once

//
// 
// █▀█ ▄▀█ ▀█▀ █▀▀ █░█   █▀█ █░█ █▄░█ █▄░█ █▀▀ █▀█
// █▀▀ █▀█ ░█░ █▄▄ █▀█   █▀▄ █▄█ █░▀█ █░▀█ ██▄ █▀▄
// Text created using https://fsymbols.com/generators/carty/
//

// #include "Sport.hpp"
#include "InputPort.hpp"
#include "OutputPort.hpp"
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
        if (module->processed) return;
        module->processed = true;

        // Push the module onto the stack of processed modules.  When the 
        // processing of the patch is complete, we'll pop all the modules
        // off the stack and reset their processed flags to false.

        processed_modules.push(module);

        std::vector<InputPort *> input_ports = module->getInputPorts();

        for (auto &input_port : input_ports)
        {
            if (input_port->isConnected())
            {
                OutputPort *connected_output_port = input_port->getConnectedOutput();

                IModule *connected_module = connected_output_port->getParentModule();

                // If the connected module is not currently being processed,
                // process it.  Otherwise, use the output from the last timestep.
                if (!connected_module->processed)
                {
                    processModule(connected_module, sample_rate);
                }
                else
                {
                    input_port->setVoltage(connected_output_port->getLastVoltage());
                }
            }
        }

        module->process(sample_rate);
    } 

    //
    // process()
    //
    //
    // This is called at sample rate.
    // TODO: Handle passing in sample rate
    //
    void process(unsigned int sample_rate, Patch *patch)
    {
        // Reset all module processed flags to false
        resetProcessedFlags();

        IModule *terminal_output_module = patch->getTerminalOutputModule();

        if(! terminal_output_module)
        {
            return;
        }

        // Compute the outputs of the system by starting with the last module,
        // then working backwards through the chain.
        processModule(terminal_output_module, sample_rate);
    }

    // Reset all module processed flags to false
    // rewrite to be recursive?  Or push traversed modules onto a stack?
    void resetProcessedFlags()
    {
        while(! processed_modules.empty())
        {
            IModule *module = processed_modules.top();
            module->processed = false;
            processed_modules.pop();
        }
    }
};