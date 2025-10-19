#pragma once

#include "ExcitationSource.hpp"
#include "../SafeMath.hpp"
#include <cmath>

// Bytebeat excitation source with multiple equations
struct BytebeatExcitation : ExcitationSource
{
    unsigned int t = 0;
    int equationIndex = 0;

    // Parameters for equations (currently fixed, could be exposed later)
    uint32_t p1 = 128;
    uint32_t p2 = 64;
    uint32_t p3 = 32;

    BytebeatExcitation()
    {
        reset();
    }

    void reset()
    {
        t = 0;
    }

    void setEquation(int index)
    {
        equationIndex = index;
    }

    void setParameters(uint32_t p1Value, uint32_t p2Value, uint32_t p3Value)
    {
        p1 = p1Value;
        p2 = p2Value;
        p3 = p3Value;
    }

    // Generate a single bytebeat sample (for bleed effect)
    float generateSingle(uint32_t timeValue)
    {
        uint32_t value = computeEquation(equationIndex, timeValue, p1, p2, p3);
        return ((value & 0xFF) / 127.5f) - 1.0f;
    }

    void generate(float *buffer, int length) override
    {
        generate(buffer, length, 0, 44100.0f, 1.0f);
    }

    void generate(float *buffer, int length, uint32_t offset, float sampleRate, float rate)
    {
        // Normalize playback rate to 44100 Hz reference
        // This ensures consistent sound across different sample rates
        // Then multiply by user rate parameter (0.25x to 8x)
        float tIncrement = (44100.0f / sampleRate) * rate;

        // Use floating-point accumulator for smooth rate scaling
        float tAccumulator = (float)offset;

        for (int i = 0; i < length; i++)
        {
            uint32_t currentT = (uint32_t)tAccumulator;
            uint32_t value = computeEquation(equationIndex, currentT, p1, p2, p3);

            // Normalize to -1.0 to 1.0 range
            buffer[i] = ((value & 0xFF) / 127.5f) - 1.0f;

            tAccumulator += tIncrement;
        }

        // Update t for continuous mode
        t = (uint32_t)tAccumulator;
    }

private:
    // Scaling function from original implementation
    uint32_t f(uint8_t value)
    {
        return (value << 4) & 0xFFF;
    }

    uint32_t computeEquation(int index, uint32_t t, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        // Ensure parameters are never zero for safety
        p1 = (p1 == 0) ? 1 : p1;
        p2 = (p2 == 0) ? 1 : p2;
        p3 = (p3 == 0) ? 1 : p3;

        switch(index)
        {
            case 0: // Alpha
                return f(
                    ((((t^(p1>>3))-456)*(p2+1))/(((SafeMath::mod(t>>(p3>>3), 14))+1)))+(t*((182>>(SafeMath::mod(t>>15, 16)))&1))
                );

            case 1: // Omega
                return f(
                    ((((t>>5)|(t<<((p3>>4)+1))>>4)*p1)-((SafeMath::div(t, ((1853>>(SafeMath::mod(t>>(p2>>4), 15))))&1))>>(SafeMath::mod(t>>12, 6))))>>4
                );

            case 2: // Widerange
                return f(
                    (((p1^(t>>(p2>>3)))-(t>>(p3>>2))-SafeMath::mod(t, (t&p2))))
                );

            case 3: // Toner
                return f(
                    ((t>>((SafeMath::mod(t>>12, (p3>>4)))))+(SafeMath::mod((p1|t), p2)))<<2
                );

            case 4: // Exploratorium
                return f(
                    ((SafeMath::mod(t, (p1+(SafeMath::mod(t, p2)))))^(t>>(p3>>5)))*2
                );

            case 5: // Melodic
                return f(
                    (t*((t>>9|t>>p1) & p2)) & (p3+5)
                );

            case 6: // Classic Downward Wiggle
                return f(
                    ((t*9&t>>4)|(t*p1&(t>>(5+(p3>>4))))|(t*3&SafeMath::div(t, p2)))-1
                );

            case 7: // Chewie
                return f(
                    (p1-(((SafeMath::div(p3+1, t))^p1)|(t^(922+p1))))*SafeMath::div((p3+1), p1)*((t+p2)>>SafeMath::mod(p2, 19))
                );

            case 8: // Buildup
                return f(
                    (((SafeMath::div(t, (p3+1))*t) % p1) & (t - SafeMath::div(t, p2)*632))
                );

            case 9: // Question / Answer
                return f(
                    ((t * ((t>>8) | (t>>p3)) & p2 & t>>8)) ^ ((t & (t>>p1)) | (t>>6))
                );

            case 10: // Sine Wave Wiggle
                return f(
                    (uint32_t)(sin((((t >> (p1>>5)) ^ (((SafeMath::mod(t, p2+1)) & ((((t >> 4) ^ (t & 64)) & t) & (SafeMath::mod(t, p2+1)))) & (p3>>5))) * 8)*0.1)*127)
                );

            case 11: // Array Cascade
            {
                static const uint32_t arr1[16] = {231, 35, 214, 124, 39, 120, 104, 229, 79, 225, 190, 149, 114, 61, 160, 77};
                static const uint32_t arr2[8] = {22, 224, 234, 146, 157, 199, 131, 79};
                uint32_t idx2 = abs((int32_t)t) % 8;
                uint32_t val = arr2[idx2] | (((t & p1) * (t * p2)) % ((((t >> (p3>>5)) ^ (t + t)) >> 1) >> 8));
                uint32_t idx1 = val % 16;
                return f(arr1[idx1] >> 1);
            }

            case 12: // Shifting Rhythm
                return f(
                    ((t >> (p1>>5)) * ((SafeMath::mod(t, p2+1)) & (t >> (p3>>5))))
                );

            case 13: // XOR Pulse
                return f(
                    ((t & p1) ^ ((SafeMath::mod(t, p2+1)) * (t >> (p3>>4))))
                );

            case 14: // Multi-Stream
                return f(
                    (SafeMath::mod(t, p1+1)) | (SafeMath::mod(t, p2+1)) | (t >> 12) | (t >> (p3>>5)) | (SafeMath::mod(t, (p1+p2)/2+1))
                );

            case 15: // Stepped Melody
            {
                static const uint32_t seq[8] = {0, 1, 0, 2, 1, 3, 2, 4};
                return f(
                    (seq[(t >> (p1>>5)) % 8] * ((t >> (p2>>5)) | (t >> 12))) + ((t * (p3>>4)) & (t >> 4))
                );
            }

            case 16: // Tan Wave
                return f(
                    (uint32_t)(tan((t >> (p1>>5))) * (p2+1) + (p3+1))
                );

            case 17: // High Frequency Buzz
                return f(
                    (((t >> (p1>>4)) | (p2>>3)) | (SafeMath::mod(t, p3+1)) * ((t * 3) & (t >> 10)))
                );

            case 18: // Fibonacci Chaos
            {
                static const uint32_t fib[8] = {1, 1, 2, 3, 5, 8, 13, 21};
                static const uint32_t scale[8] = {16, 32, 48, 64, 80, 96, 112, 128};
                return f(
                    ((SafeMath::mod(t, p1+1)) | (t >> (p2>>5)) | (t >> 4) | ((((t >> 4) | 16) & scale[(t >> 16) % 8]) | ((uint32_t)(tan((t >> 2)) * 127 + 127) * (fib[(t >> 3) % 8] & ((t >> 4) & (SafeMath::mod(t, p3+1)))))))
                );
            }

            case 19: // Sine Chaos
                return f(
                    ((uint32_t)(sin((t >> (p1>>6))) * 127 + 127) | (abs((int32_t)(t >> 10)) | ((t & (p2>>4)) * (t >> 4)))) ^ (t >> 16) | (SafeMath::mod(t, p3+1))
                );

            case 20: // Layered Harmonics
                return f(
                    ((SafeMath::mod(t, p1+1)) | ((t >> (p3>>5)) + (p2>>4)) | (t >> 8) ^ ((t >> 10) * ((SafeMath::mod(t, p2+1)) + (t >> 4))))
                );

            case 21: // Binary Tree
            {
                static const uint32_t powers[8] = {1, 3, 7, 15, 31, 63, 127, 255};
                return f(
                    ((((t >> (p1>>4)) & (p2>>3)) | ((SafeMath::mod(t, p3+1)) & (t >> 6))) ^ ((powers[(t >> 14) % 8] & ((t >> 8) + (t & 34))) | ((t >> 10) & 32) | (SafeMath::mod(t, p1+1)) | (SafeMath::mod(t, p2+1))))
                );
            }

            default:
                return f(t * ((t >> 12) | (t >> 8)) & (63 & (t >> 4)));
        }
    }
};
