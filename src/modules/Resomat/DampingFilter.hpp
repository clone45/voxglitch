#pragma once

// Simple one-pole lowpass filter for damping the Karplus-Strong feedback loop
struct DampingFilter
{
    float z = 0.0;
    float a0 = 1.0;
    float b1 = 0.0;

    DampingFilter()
    {
        reset();
    }

    void reset()
    {
        z = 0.0;
    }

    // Set cutoff frequency
    // freq: cutoff frequency in Hz
    // sampleRate: sample rate in Hz
    void setCutoff(float freq, float sampleRate)
    {
        float x = exp(-2.0 * M_PI * freq / sampleRate);
        a0 = 1.0 - x;
        b1 = x;
    }

    // Process a single sample
    float process(float input)
    {
        z = (input * a0) + (z * b1);
        return z;
    }
};
