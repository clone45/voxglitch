#pragma once
#include <random>

// WARNING: This is barely random.  It repeats at 10,000 calls.  Good if you 
// only need a random number every so often!

struct FastRandom
{
    unsigned int read_head = 0;
    std::array<float, 10000> random_numbers_array{};
    std::uniform_real_distribution<float> dist;

    FastRandom()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        dist = std::uniform_real_distribution<float>(0.0, 1.0);

        // std::random_device is supposedly fairly slow, but since this is in the
        // contructor, I don't think it should be an issue.
        std::generate(random_numbers_array.begin(), random_numbers_array.end(), [&] () { return dist(gen); });
        read_head = rand()%(10000 + 1);
    }

    float gen()
    {
        if(++read_head >= 10000) read_head = 0;
        return(random_numbers_array.at(read_head));
    }
};
