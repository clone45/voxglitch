#pragma once
#include "../BaseModule.hpp"
#include <algorithm>

class DelayLineModule : public BaseModule {
public:
    enum Inputs {
        INPUT,
        DELAY_TIME,
        NUM_INPUTS
    };

    enum Outputs {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum Params {
        NUM_PARAMS
    };

    DelayLineModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        delay_time = 0.1; // Default delay time in seconds

        // Initialize the delay buffer with zeros
        delay_buffer.resize(max_delay_samples, 0.0);
    }

    void process(unsigned int sample_rate) override {
        float input = inputs[INPUT]->getVoltage();
        float delay_time_input = inputs[DELAY_TIME]->getVoltage();

        // Convert the delay time input to the desired range
        float delay_time_seconds = map(delay_time_input, -5.0, 5.0, 0.001, 1.0);

        // Calculate the delay time in samples
        unsigned int delay_samples = delay_time_seconds * sample_rate;

        // Restrict the delay samples within the maximum buffer size
        delay_samples = std::min(delay_samples, max_delay_samples);

        // Read the delayed sample from the buffer
        float delayed_sample = delay_buffer[read_index];

        // Store the current input sample in the buffer
        delay_buffer[write_index] = input;

        // Increment the read and write indices
        read_index++;
        write_index++;

        // Wrap the indices around if they exceed the buffer size
        if (read_index >= delay_samples)
            read_index = 0;
        if (write_index >= delay_samples)
            write_index = 0;

        // Output the delayed sample
        outputs[OUTPUT]->setVoltage(delayed_sample);
    }

private:
    static constexpr unsigned int max_delay_samples = 44100; // Maximum delay time of 1 second at 44.1 kHz sample rate
    std::vector<float> delay_buffer;
    unsigned int read_index = 0;
    unsigned int write_index = 0;
    float delay_time;
};
