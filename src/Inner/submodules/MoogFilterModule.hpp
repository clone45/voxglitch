/*
#pragma once

#include <cmath>
#include "../BaseModule.hpp"

class MoogFilterModule : public BaseModule {
public:
    MoogFilterModule() {
        // Add input and output ports
        addInput(std::make_shared<Input>(this));
        addOutput(std::make_shared<Output>(this));
        addParameter("cutoff", 1000.0f);
        addParameter("resonance", 0.0f);
    }

    void update() override {
        // Get input value
        float input = getInputValue(getInputPort(0));

        // Get parameters
        float cutoff = std::any_cast<float>(getParameter("cutoff"));
        float resonance = std::any_cast<float>(getParameter("resonance"));

        // Calculate filter coefficients
        float f = cutoff / (0.5f * 44100.0f);
        float k = 3.6f * (f - 0.45f);
        float p = (k + 1.0f) * 0.5f;
        float scale = exp((1.0f - p) * 1.386249f);

        float a0 = 1.0f;
        float a1 = -1.0f * (k + 1.0f) / (2.0f * scale);
        float a2 = k / (2.0f * scale);
        float a3 = a2 / (2.0f * scale);
        float a4 = a2 * a2 / a0;

        // Calculate output
        float out = a0 * input - a1 * state[0] - a2 * state[1] - a3 * state[2] - a4 * state[3];
        state[3] = state[2];
        state[2] = state[1];
        state[1] = state[0];
        state[0] = out;

        // Set output value
        setOutputValue(getOutputPort(0), out);
    }

    float getInputValue(std::shared_ptr<Input> input) const override {
        return input->getValue();
    }

    void setOutputValue(std::shared_ptr<Output> output, float value) override {
        const auto& connected_inputs = output->getConnectedToInputs();
        for (const auto& input : connected_inputs) {
            input->setValue(value);
        }
    }

private:
    float state[4] = { 0.0f };
};
*/