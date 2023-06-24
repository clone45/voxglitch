#pragma once

#include "../BaseModule.hpp"
#include "C:/Code/bonsaiyo/includes/VPlugin.hpp"

class ProxyModule : public BaseModule
{
public:

    VPlugin *plugin = nullptr;
    json_t* data = nullptr;

    std::vector<float> input_voltages;
    std::vector<float> output_voltages;

    unsigned int num_inputs = 0;
    unsigned int num_outputs = 0;

    ProxyModule(int num_inputs, int num_outputs, json_t* data, VPlugin *plugin)
    {
        this->plugin = plugin;
        this->data = data;
        config(0, num_inputs, num_outputs);

        // Set the size of input_voltages and output_voltages
        input_voltages.resize(num_inputs);
        output_voltages.resize(num_outputs);

        this->num_inputs = num_inputs;
        this->num_outputs = num_outputs;
    } 

    void process(unsigned int sample_rate) override
    {
        // input and outputs are defined in IModule.h as
        //  std::vector<Sport*> inputs;
        //  std::vector<Sport*> outputs;

        // Note: If performance was not a concern, I would use getters and setters
        // for inputs and outputs.  However, I'm trying to avoid the overhead of
        // function calls.

        for (unsigned int i = 0; i < num_inputs; i++) 
        {
            input_voltages[i] = inputs[i]->voltage;
        }

        //
        // Possible speed improvement might be to pass in the outputs vector
        // by reference, and have the plugin write directly to it.  Could I do that
        // for the inputs vector as well?

        // Call process on the Plugin DLL, passing in the voltages
        output_voltages = plugin->process(sample_rate, input_voltages);

        // For each output, set the voltage of this module's outputs
        for (unsigned int i = 0; i < num_outputs; i++) 
        {
            // outputs[i]->setVoltage(output_voltages[i]);
            outputs[i]->voltage = output_voltages[i];
        }
    }
};
