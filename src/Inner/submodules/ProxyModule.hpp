#pragma once

#include "../BaseModule.hpp"
#include "C:/Code/bonsaiyo/includes/VPlugin.hpp"

class ProxyModule : public BaseModule
{
public:

    VPlugin *plugin = nullptr;
    json_t* data = nullptr;

    ProxyModule(int num_inputs, int num_outputs, json_t* data, VPlugin *plugin)
    {
        this->plugin = plugin;
        this->data = data;
        config(0, num_inputs, num_outputs);
    } 

    void process(unsigned int sample_rate) override
    {
        std::vector<float> input_voltages;
        std::vector<float> output_voltages;

        // Store the input voltages in a vector that will be passed
        // into the process function of the Plugin DLL
        for (int i = 0; i < this->getNumInputs(); i++) 
        {
            input_voltages.push_back(inputs[i]->getVoltage());
        }

        // Call process on the Plugin DLL, passing in the voltages
        // TODO: pass in sample rate
        output_voltages = plugin->process(sample_rate, input_voltages);

        // For each output, set the voltage of this module's outputs
        for (int i = 0; i < this->getNumOutputs(); i++) 
        {
            outputs[i]->setVoltage(output_voltages[i]);
        }
    }
};
