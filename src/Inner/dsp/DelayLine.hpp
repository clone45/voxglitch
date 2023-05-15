#pragma once
#include <vector>

class DelayLine
{
public:
    DelayLine(float max_delay_time, unsigned sample_rate)
        : max_delay_samples(max_delay_time * sample_rate)
    {
        buffer.resize(max_delay_samples);
        write_pos = 0;
    }

    void set_delay_time(float delay_time, unsigned sample_rate)
    {
        float delay_samples_f = delay_time * sample_rate;
        delay_samples_int = static_cast<int>(delay_samples_f);
        delay_frac = delay_samples_f - delay_samples_int;
        read_pos = (write_pos - delay_samples_int + max_delay_samples) % max_delay_samples;
    }

    float process(float input)
    {
        float output;

        int next_read_pos = (read_pos + 1) % max_delay_samples;
        output = (1.0 - delay_frac) * buffer[read_pos] + delay_frac * buffer[next_read_pos];
        buffer[write_pos] = input;

        read_pos = next_read_pos;
        write_pos = (write_pos + 1) % max_delay_samples;

        return output;
    }

private:
    std::vector<float> buffer;
    int max_delay_samples;
    int delay_samples_int;
    float delay_frac;
    int read_pos;
    int write_pos;
};
