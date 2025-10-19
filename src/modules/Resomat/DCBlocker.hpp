#pragma once

// DC blocking filter to remove DC offset from the signal
// Uses a simple high-pass filter
struct DCBlocker
{
    float xPrev = 0.0;
    float yPrev = 0.0;
    float alpha = 0.995;

    DCBlocker()
    {
        reset();
    }

    void reset()
    {
        xPrev = 0.0;
        yPrev = 0.0;
    }

    void setAlpha(float a)
    {
        alpha = clamp(a, 0.9f, 0.9999f);
    }

    float process(float input)
    {
        float output = input - xPrev + alpha * yPrev;
        xPrev = input;
        yPrev = output;
        return output;
    }
};
