#pragma once

#include <cmath>

class OnePoleFilter
{
public:
    OnePoleFilter(float cutoff_freq, float sample_rate)
    {
        setCoefficients(cutoff_freq, sample_rate);
        reset();
    }

    void setCoefficients(float cutoff_freq, float sample_rate)
    {
        float omega = 2.0f * 3.14159265358979323846f * cutoff_freq / sample_rate;
        a0 = 1.0f - std::exp(-omega);
        b1 = -std::exp(-omega);
    }

    void reset()
    {
        y1 = 0.0f;
    }

    float process(float x)
    {
        float y = a0 * x + b1 * y1;
        y1 = y;
        return y;
    }

private:
    float a0, b1;
    float y1;
};
