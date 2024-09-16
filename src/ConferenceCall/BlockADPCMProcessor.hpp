// src/ConferenceCall/BlockADPCMProcessor.hpp
#pragma once

class BlockADPCMProcessor
{
public:
    static const int BLOCK_SIZE = 64;
    static const int STEP_TABLE_SIZE = 89;
    static const int INDEX_TABLE_SIZE = 16;

    BlockADPCMProcessor()
    {
        stepIndex = 0;
        predictor = 0;
        initializeTables();
    }

    std::vector<uint8_t> compressBlock(const std::vector<float>& input_buffer)
    {
        std::vector<uint8_t> compressed;
        compressed.reserve((input_buffer.size() + 1) / 2 + 4);  // +4 for header

        // Write header
        int16_t initialPredictor = static_cast<int16_t>(input_buffer[0] * 32767.0f);
        compressed.push_back(initialPredictor & 0xFF);
        compressed.push_back((initialPredictor >> 8) & 0xFF);
        compressed.push_back(static_cast<uint8_t>(stepIndex));
        compressed.push_back(0);  // Reserved

        predictor = static_cast<float>(initialPredictor) / 32767.0f;

        for (size_t i = 1; i < input_buffer.size(); i += 2)
        {
            uint8_t nibble1 = encodeSample(input_buffer[i]);
            uint8_t nibble2 = (i + 1 < input_buffer.size()) ? encodeSample(input_buffer[i + 1]) : 0;
            compressed.push_back((nibble1 << 4) | nibble2);
        }

        return compressed;
    }

    std::vector<float> decompressBlock(const std::vector<uint8_t>& compressed)
    {
        std::vector<float> output;
        output.reserve(BLOCK_SIZE);

        if (compressed.size() < 4) return output;  // Invalid block

        // Read header
        int16_t initialPredictor = static_cast<int16_t>(compressed[0] | (compressed[1] << 8));
        stepIndex = compressed[2];
        predictor = static_cast<float>(initialPredictor) / 32767.0f;

        output.push_back(predictor);

        for (size_t i = 4; i < compressed.size(); ++i)
        {
            output.push_back(decodeSample((compressed[i] >> 4) & 0x0F));
            if (output.size() >= BLOCK_SIZE) break;

            output.push_back(decodeSample(compressed[i] & 0x0F));
            if (output.size() >= BLOCK_SIZE) break;
        }

        return output;
    }

private:
    int stepIndex;
    float predictor;

    std::vector<int> stepTable;
    static const int8_t indexTable[INDEX_TABLE_SIZE];

    void initializeTables()
    {
        stepTable = {
            7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
            50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230,
            253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963,
            1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
            3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487,
            12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
        };
    }

    uint8_t encodeSample(float sample)
    {
        int step = stepTable[stepIndex];
        int diff = static_cast<int>((sample - predictor) * 32767.0f);
        
        uint8_t nibble = 0;
        if (diff < 0) {
            nibble = 8;
            diff = -diff;
        }

        int delta = 0;
        for (int i = 4; i > 0; i >>= 1) {
            if (diff >= step) {
                nibble |= i;
                diff -= step;
                delta += step;
            }
            step >>= 1;
        }

        predictor += (nibble & 8) ? -delta / 32767.0f : delta / 32767.0f;
        predictor = std::max(-1.0f, std::min(1.0f, predictor));

        stepIndex = std::max(0, std::min(STEP_TABLE_SIZE - 1, stepIndex + indexTable[nibble]));

        return nibble;
    }

    float decodeSample(uint8_t nibble)
    {
        int step = stepTable[stepIndex];
        int delta = 0;
        if (nibble & 4) delta += step;
        if (nibble & 2) delta += step >> 1;
        if (nibble & 1) delta += step >> 2;
        delta += step >> 3;

        if (nibble & 8)
            predictor -= static_cast<float>(delta) / 32767.0f;
        else
            predictor += static_cast<float>(delta) / 32767.0f;

        predictor = std::max(-1.0f, std::min(1.0f, predictor));
        stepIndex = std::max(0, std::min(STEP_TABLE_SIZE - 1, stepIndex + indexTable[nibble]));

        return predictor;
    }
};

const int8_t BlockADPCMProcessor::indexTable[BlockADPCMProcessor::INDEX_TABLE_SIZE] = {
    -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8
};