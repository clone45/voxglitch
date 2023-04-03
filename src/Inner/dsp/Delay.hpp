#pragma once

#include <vector>

class Delay
{
public:
    Delay(int max_delay_samples)
        : max_delay_samples(max_delay_samples)
    {
        buffer.resize(max_delay_samples);
        write_pos = 0;
        delay_samples = 0;
    }

    void set_delay_samples(int delay_samples)
    {
        this->delay_samples = delay_samples;
        read_pos = (write_pos - delay_samples + max_delay_samples) % max_delay_samples;
    }

    float process(float input)
    {
        float output = buffer[read_pos];
        buffer[write_pos] = input;

        read_pos = (read_pos + 1) % max_delay_samples;
        write_pos = (write_pos + 1) % max_delay_samples;

        return output;
    }

private:
    std::vector<float> buffer;
    int max_delay_samples;
    int delay_samples;
    int read_pos;
    int write_pos;
};