#pragma once
#include <cmath>
#include <random>
#include "../../vgLib-2.0/helpers/Debugging.hpp"
#include "KaisekiSamplePlayer.hpp"

class KaisekiTimeStretch {
private:
    static const int MAX_GRAINS = 64;
    
    struct Grain {
        double readPosition = 0.0f;
        float phase = 0.0f;
        bool active = false;
        
        void start(double position) {
            readPosition = position;
            phase = 0.0f;
            active = true;
        }
    };
    
    Grain grains[MAX_GRAINS];
    
    float sourceReadHead = 0.0f;
    double nextGrainTime = 0.0;
    double outputTime = 0.0;
    
    float grainSizeSamples = 4096.0f;
    float overlapRatio = 0.75f;
    int grainCount = 4;
    double stretchFactor = 1.0f;
    
    float sourceBPM = 120.0f;
    float targetBPM = 120.0f;
    float pitchMultiplier = 1.0f;
    float positionOffset = 0.0f;
    bool offsetPending = false;
    
    // Random number generation for jitter
    std::mt19937 rng;
    std::uniform_real_distribution<float> jitterDist;
    
    // Simple Hann window function
    float calculateHannWindow(float phase) {
        if (phase < 0.0f || phase >= 1.0f) return 0.0f;
        return 0.5f * (1.0f - cosf(2.0f * M_PI * phase));
    }
    
    std::pair<float, float> readSample(KaisekiSamplePlayer& source, float position) {
        if (!source.isLoaded()) return {0.0f, 0.0f};
        
        // Use unsigned int for sampleSize and pos
        unsigned int sampleSize = source.sample.size(); // Sample size should be non-negative
        if (sampleSize == 0) return {0.0f, 0.0f};

        while (position >= sampleSize) position -= sampleSize;
        while (position < 0) position += sampleSize; // This can be replaced with wrap-around logic

        unsigned int pos = (unsigned int)position;  // Use unsigned int for position

        // Read samples without interpolation
        float leftSample = 0.0f, rightSample = 0.0f;
        source.sample.read(pos, &leftSample, &rightSample);

        return {leftSample, rightSample};
    }

    std::pair<float, float> readSampleLinearInterp(KaisekiSamplePlayer& source, float position) {
        if (!source.isLoaded()) return {0.0f, 0.0f};
        
        int sampleSize = source.sample.size();
        if (sampleSize == 0) return {0.0f, 0.0f};
        
        while (position >= sampleSize) position -= sampleSize;
        while (position < 0) position += sampleSize;
        
        int pos1 = (int)position;
        int pos2 = (pos1 + 1) % sampleSize;
        float frac = position - pos1;
        
        float leftSample1 = 0.0f, rightSample1 = 0.0f;
        float leftSample2 = 0.0f, rightSample2 = 0.0f;
        
        source.sample.read(pos1, &leftSample1, &rightSample1);
        source.sample.read(pos2, &leftSample2, &rightSample2);
        
        float leftInterp = leftSample1 + frac * (leftSample2 - leftSample1);
        float rightInterp = rightSample1 + frac * (rightSample2 - rightSample1);
        
        return {leftInterp, rightInterp};
    }

    
public:
    KaisekiTimeStretch() : rng(std::random_device{}()), jitterDist(-0.5f, 0.5f) {
        reset();
    }
    
    void reset() {
        for (int i = 0; i < MAX_GRAINS; i++) {
            grains[i].active = false;
        }
        sourceReadHead = 0.0f;
        nextGrainTime = 0.0;
        outputTime = 0.0;
    }
    
    void setPosition(float position) {
        sourceReadHead = position;
        // Clear all active grains for immediate position change
        // This ensures we hear audio from the new position immediately
        for (int i = 0; i < MAX_GRAINS; i++) {
            grains[i].active = false;
        }
        
        // Reset timing so new grains spawn immediately at the new position
        nextGrainTime = outputTime - 1.0f;  // Ensure next grain spawns immediately
    }
    
    void setSourceBPM(float bpm) {
        sourceBPM = rack::math::clamp(bpm, 20.0f, 300.0f);
        updateStretchFactor();
    }
    
    void setTargetBPM(float bpm) {
        targetBPM = rack::math::clamp(bpm, 20.0f, 300.0f);
        updateStretchFactor();
    }
    
    void setGrainSize(float sizeMs) {
        grainSizeSamples = (sizeMs / 1000.0f) * APP->engine->getSampleRate();
        grainSizeSamples = rack::math::clamp(grainSizeSamples, 256.0f, 16384.0f);
    }
    
    void setOverlapRatio(float ratio) {
        overlapRatio = rack::math::clamp(ratio, 0.25f, 0.95f);
    }
    
    void setGrainCount(int count) {
        // Knob is present but ignored - grain count is now adaptive
        // grainCount = rack::math::clamp(count, 2, MAX_GRAINS);
    }
    
    void setPitchMultiplier(float multiplier) {
        pitchMultiplier = rack::math::clamp(multiplier, 0.25f, 4.0f);
    }

    void setOffset(float offset) {
        float clamped = rack::math::clamp(offset, 0.0f, 1.0f);
        VBUG << "TimeStretch setOffset: " << positionOffset << " -> " << clamped;
        positionOffset = clamped;
        offsetPending = true;
    }

    std::pair<float, float> process(KaisekiSamplePlayer& source) {
        if (!source.isLoaded()) return {0.0f, 0.0f};

        double hopSize = grainSizeSamples * (1.0f - overlapRatio);

        if (offsetPending) {
            unsigned int sampleSize = source.sample.size();
            if (sampleSize > 0) {
                float maxIndex = static_cast<float>(sampleSize - 1);
                float newReadHead = rack::math::clamp(positionOffset * sampleSize, 0.0f, maxIndex);
                sourceReadHead = newReadHead;

                for (int i = 0; i < MAX_GRAINS; i++) {
                    grains[i].active = false;
                }

                nextGrainTime = outputTime - 1.0f;
            }
            offsetPending = false;
        }

        // Calculate adaptive grain count based on stretch factor and desired overlap
        // Formula: we need enough grains so that (grainSize / hopSize) * stretchFactor grains are active
        // Since we now divide by stretchFactor for timing, we multiply here for grain count
        int adaptiveGrainCount = (int)ceil((grainSizeSamples / hopSize) * stretchFactor);
        adaptiveGrainCount = rack::math::clamp(adaptiveGrainCount, 2, MAX_GRAINS);
        grainCount = adaptiveGrainCount;
        
        if (outputTime >= nextGrainTime) {
            // Find an inactive grain to start
            for (int i = 0; i < grainCount; i++) {
                if (!grains[i].active) {
                    // Apply current read head (updated when offsetPending was set)
                    unsigned int sampleSize = source.sample.size();
                    float offsetReadHead = sourceReadHead;

                    // Wrap around if offset pushes us beyond sample boundaries
                    while (offsetReadHead >= sampleSize) offsetReadHead -= sampleSize;
                    while (offsetReadHead < 0.0f) offsetReadHead += sampleSize;

                    grains[i].start(offsetReadHead);
                    break;
                }
            }
            
            // Analysis head advances at 1× (preserves pitch cleanly)
            double sampleRateRatio = (source.sample.sample_rate > 0) ? (source.sample.sample_rate / APP->engine->getSampleRate()) : 1.0f;
            
            // Add small jitter to source read head to prevent artifacts
            float jitter = jitterDist(rng);
            sourceReadHead += (hopSize * sampleRateRatio) + jitter;
            
            unsigned int sampleSize = source.sample.size();
            if (sourceReadHead >= sampleSize) {
                sourceReadHead = fmodf(sourceReadHead, sampleSize);
            }
            
            // Synthesis hop carries the stretch - divide to make higher BPM = faster playback
            nextGrainTime += hopSize / stretchFactor;
        }
        
        float leftOutput = 0.0f;
        float rightOutput = 0.0f;
        float windowSum = 0.0f;
        
        // Process all active grains
        for (int i = 0; i < grainCount; i++) {
            if (grains[i].active) {
                
                // Read stereo samples (using pair return for efficiency)
                std::pair<float, float> samples = readSampleLinearInterp(source, grains[i].readPosition);
                float leftSample = samples.first;
                float rightSample = samples.second;
                // std::pair<float, float> samples = readSample(source, grains[i].readPosition);

                // Apply Hann window
                float window = calculateHannWindow(grains[i].phase);
                
                leftOutput += leftSample * window;
                rightOutput += rightSample * window;
                windowSum += window;
                
                // Advance grain (with pitch multiplier applied)
                double sampleRateRatio = (source.sample.sample_rate > 0) ?  (source.sample.sample_rate / APP->engine->getSampleRate()) : 1.0f;
                grains[i].readPosition += sampleRateRatio * pitchMultiplier;
                grains[i].phase += 1.0f / grainSizeSamples;
                
                if (grains[i].phase >= 1.0f) {
                    grains[i].active = false;
                }
            }
        }
        
        // Normalize by window sum
        if (windowSum > 0.0f) {
            float norm = 1.0f / windowSum;
            leftOutput *= norm;
            rightOutput *= norm;
        }
        
        outputTime += 1.0f;
        
        return {leftOutput, rightOutput};
    }
    
    float getStretchFactor() const {
        return stretchFactor;
    }
    
    float getSourceReadHead() const {
        return sourceReadHead;
    }
    
private:
    void updateStretchFactor() {
        if (sourceBPM > 0.0f) {
            stretchFactor = targetBPM / sourceBPM;
            stretchFactor = rack::math::clamp(stretchFactor, 0.25f, 4.0f);
        }
    }
};
