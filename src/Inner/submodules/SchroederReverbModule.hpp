#pragma once

#include <vector>
#include "../BaseModule.hpp"

class SchroederReverbModule : public BaseModule {
private:
    std::vector<float> delays{ 341, 613, 1557, 2373 };
    std::vector<float> gains{ 0.1f, 0.2f, 0.4f, 0.8f };
    std::vector<std::vector<float>> buffers{
        std::vector<float>(static_cast<int>(delays[0]), 0.0f),
        std::vector<float>(static_cast<int>(delays[1]), 0.0f),
        std::vector<float>(static_cast<int>(delays[2]), 0.0f),
        std::vector<float>(static_cast<int>(delays[3]), 0.0f)
    };

public:
    Sport *input_port = new Sport(this);
    Sport *output_port = new Sport(this);
    Sport *mix_input_port = new Sport(this);
    Sport *decay_input_port = new Sport(this);

    SchroederReverbModule() 
    {
        setParameter("mix", 0.5f);
        setParameter("decay", 0.5f);
    }

    void process(unsigned int sample_rate) override
    {
        float input = input_port->isConnected() ? input_port->getValue() : 0.0f;
        float mix = mix_input_port->isConnected() ? (mix_input_port->getValue() / 10.0)  : getParameter("mix");
        float decay = decay_input_port->isConnected() ? (decay_input_port->getValue() / 10.0) : getParameter("decay");

        float output = input;

        for (size_t i = 0; i < delays.size(); i++) {
            float gain = gains[i] * decay;

            int buffer_index = static_cast<int>(i) % static_cast<int>(buffers.size());

            float delayed_sample = buffers[buffer_index][0];
            output += gain * delayed_sample;

            for (size_t j = 0; j < buffers[buffer_index].size() - 1; j++) {
                buffers[buffer_index][j] = buffers[buffer_index][j + 1];
            }

            buffers[buffer_index][buffers[buffer_index].size() - 1] = output;
        }

        output = mix * output + (1.0f - mix) * input;
        output_port->setValue(output);
    }


    Sport *getPortByName(std::string port_name) override 
    {
        if (port_name == "INPUT_PORT") {
            return input_port;
        }
        else if (port_name == "OUTPUT_PORT") {
            return output_port;
        }
        else if (port_name == "MIX_INPUT_PORT") {
            return mix_input_port;
        }
        else if (port_name == "DECAY_INPUT_PORT") {
            return decay_input_port;
        }
        else {
            return nullptr;
        }
    }

    std::vector<Sport *> getInputPorts() override 
    {
        return { input_port, mix_input_port, decay_input_port };
    }

    std::vector<Sport *> getOutputPorts() override 
    {
        return { output_port };
    }
};