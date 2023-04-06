#pragma once

#include <vector>
#include "../BaseModule.hpp"

class SchroederReverbModule : public BaseModule 
{

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

    enum INPUTS {
        AUDIO,
        MIX,
        DECAY,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        MIX,
        DECAY,
        NUM_PARAMS
    };

    SchroederReverbModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        float input = inputs[AUDIO]->isConnected() ? inputs[AUDIO]->getVoltage() : 0.0f;
        float mix = inputs[MIX]->isConnected() ? (inputs[MIX]->getVoltage() / 10.0)  : params[MIX].getValue();
        float decay = inputs[DECAY]->isConnected() ? (inputs[DECAY]->getVoltage() / 10.0) : params[DECAY].getValue();

        float output = input;

        for (size_t i = 0; i < delays.size(); i++) 
        {
            float gain = gains[i] * decay;

            int buffer_index = static_cast<int>(i) % static_cast<int>(buffers.size());

            float delayed_sample = buffers[buffer_index][0];
            output += gain * delayed_sample;

            for (size_t j = 0; j < buffers[buffer_index].size() - 1; j++) 
            {
                buffers[buffer_index][j] = buffers[buffer_index][j + 1];
            }

            buffers[buffer_index][buffers[buffer_index].size() - 1] = output;
        }

        output = mix * output + (1.0f - mix) * input;
        outputs[OUTPUT]->setValue(output);
    }
};
