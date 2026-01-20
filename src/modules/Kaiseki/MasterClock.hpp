#pragma once

class MasterClock {
private:
    double position = 0.0;          // 0.0 to 1.0 using double for precision
    double samplesPerLoop = 0;      // Total samples in one loop at current sample rate
    float masterBPM = 0.0f;         // BPM of first loaded sample
    int beatsPerLoop = 4;           // Assume 4 beats (1 bar) by default
    bool isActive = false;          // Clock starts when first sample loads
    float currentSampleRate = 48000.0f;

public:
    MasterClock() {}

    void initFromFirstSample(float bpm, float sampleRate) {
        masterBPM = bpm;
        currentSampleRate = sampleRate;
        double beatsPerSecond = bpm / 60.0;
        double secondsPerLoop = beatsPerLoop / beatsPerSecond;
        samplesPerLoop = secondsPerLoop * sampleRate;
        isActive = true;
        position = 0.0;
    }

    void updateSampleRate(float sampleRate) {
        if (!isActive) return;

        currentSampleRate = sampleRate;
        double beatsPerSecond = masterBPM / 60.0;
        double secondsPerLoop = beatsPerLoop / beatsPerSecond;

        // Preserve position when changing sample rate
        double oldProgress = position;
        samplesPerLoop = secondsPerLoop * sampleRate;
        position = oldProgress;
    }

    bool step() {
        if (!isActive) return false;

        position += 1.0 / samplesPerLoop;
        if (position >= 1.0) {
            position -= 1.0;
            return true; // Loop boundary!
        }
        return false;
    }

    void reset() {
        position = 0.0;
        isActive = false;
        masterBPM = 0.0f;
        samplesPerLoop = 0;
    }

    void stop() {
        isActive = false;
    }

    void start() {
        if (masterBPM > 0) {
            isActive = true;
        }
    }

    double getPosition() const {
        return position;
    }

    float getMasterBPM() const {
        return masterBPM;
    }

    bool getIsActive() const {
        return isActive;
    }

    int getBeatsPerLoop() const {
        return beatsPerLoop;
    }

    void setBeatsPerLoop(int beats) {
        beatsPerLoop = beats;
        if (isActive) {
            // Recalculate samples per loop
            double beatsPerSecond = masterBPM / 60.0;
            double secondsPerLoop = beatsPerLoop / beatsPerSecond;
            samplesPerLoop = secondsPerLoop * currentSampleRate;
        }
    }

    void setMasterBPM(float bpm) {
        masterBPM = bpm;
        if (isActive) {
            // Recalculate samples per loop with new BPM
            double beatsPerSecond = masterBPM / 60.0;
            double secondsPerLoop = beatsPerLoop / beatsPerSecond;
            samplesPerLoop = secondsPerLoop * currentSampleRate;
        }
    }

    double getLoopLengthInSamples() const {
        return samplesPerLoop;
    }
};