#pragma once
#include <random>

struct Random
{
    std::uniform_real_distribution<float> dist = std::uniform_real_distribution<float>(0.0, 1.0);
    std::random_device rd;

    Random()
    {
    }

    float gen()
    {
        std::mt19937 gen(rd());
        return dist(gen);
    }
};
