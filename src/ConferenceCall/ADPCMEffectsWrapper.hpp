#pragma once
#include "BlockADPCMProcessor.hpp"
#include <vector>
#include <random>

class ADPCMEffectsWrapper {
public:
    ADPCMEffectsWrapper(BlockADPCMProcessor& adpcm) 
        : adpcm(adpcm), rng(std::random_device{}()), 
          dropoutDist(0.0f, 1.0f) {}

    void setDropoutAmount(float amount) { dropoutAmount = amount; }

    std::vector<uint8_t> processAndCompress(const std::vector<float>& input) {
        std::vector<float> processed = applyPreEffects(input);
        return adpcm.compressBlock(processed);
    }

    std::vector<float> decompressAndProcess(const std::vector<uint8_t>& compressed) {
        std::vector<float> decompressed = adpcm.decompressBlock(compressed);
        return applyPostEffects(decompressed);
    }

private:
    BlockADPCMProcessor& adpcm;
    float dropoutAmount = 0.0f;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dropoutDist;

    std::vector<float> applyPreEffects(const std::vector<float>& input) {
        std::vector<float> processed = input;

        for (float& sample : processed) {
            if (dropoutDist(rng) < dropoutAmount) {
                sample = 0.0f;  // Drop the sample
            }
        }

        return processed;
    }

    std::vector<float> applyPostEffects(const std::vector<float>& decompressed) {
        return decompressed;  // No post-effects for now
    }
};