#pragma once

#include "ExcitationSource.hpp"

// White noise excitation source
// Generates random values between -1.0 and 1.0
struct NoiseExcitation : ExcitationSource
{
    void generate(float *buffer, int length) override
    {
        for (int i = 0; i < length; i++)
        {
            buffer[i] = (random::uniform() * 2.0f - 1.0f);
        }
    }
};
