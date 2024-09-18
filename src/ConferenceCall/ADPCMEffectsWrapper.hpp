#pragma once
#include "BlockADPCMProcessor.hpp"
#include <vector>

class ADPCMEffectsWrapper {
public:
    ADPCMEffectsWrapper(BlockADPCMProcessor& adpcm) : adpcm(adpcm) {}

    std::vector<uint8_t> compress(const std::vector<float>& input) {
        return adpcm.compressBlock(input);
    }

    std::vector<float> decompress(const std::vector<uint8_t>& compressed) {
        return adpcm.decompressBlock(compressed);
    }

private:
    BlockADPCMProcessor& adpcm;
};