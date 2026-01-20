#pragma once

#include <string>
#include <regex>
#include <cmath>
#include "OSCScaling.hpp"

// Enhanced OSC message handler for multi-track control
class KaisekiMessageHandler {
public:
    enum class MixMode {
        SELECT,
        CROSSFADE,
        MULTI_SELECT
    };
    
    struct TrackControl {
        float pitch = 0.0f;
        float volume = 1.0f;
        float pan = 0.0f;
        int cuePoint = 0;
        int loopMode = 2; // 0=one-shot, 1=finite, 2=infinite
        int loopCount = 1;
        bool trigger = false;
        bool cueTrigger = false;
    };
    
    // Global controls
    float globalBPM = 120.0f;
    float grainSize = 175.0f; // ms
    float grainOverlap = 0.5f;
    int selectedPack = 0;
    int selectedTracks[8] = {0, 1, 2, 3, 4, 5, 6, 7}; // Track indices for each slot
    MixMode mixMode = MixMode::MULTI_SELECT;
    float mixLevel = 0.0f; // For SELECT and CROSSFADE modes
    float mixLevels[8] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}; // For MULTI_SELECT
    
    // Per-track controls
    TrackControl trackControls[8];
    
    // Per-track grain controls (optional)
    float trackGrainSize[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // 0 = use global
    float trackGrainOverlap[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // 0 = use global
    
    KaisekiMessageHandler() {
        reset();
    }
    
    void reset() {
        globalBPM = 120.0f;
        grainSize = 175.0f;
        grainOverlap = 0.5f;
        selectedPack = 0;
        mixMode = MixMode::MULTI_SELECT;
        mixLevel = 0.0f;
        
        for (int i = 0; i < 8; i++) {
            selectedTracks[i] = i;
            mixLevels[i] = 1.0f;
            trackControls[i] = TrackControl();
            trackGrainSize[i] = 0.0f;
            trackGrainOverlap[i] = 0.0f;
        }
    }
    
    // Parse and handle OSC message
    bool handleMessage(const std::string& address, float value) {
        // Global BPM control
        if (address == "/simpleosc/bpm") {
            int bpm = static_cast<int>(std::round(value));
            if (bpm < 20) bpm = 20;
            if (bpm > 240) bpm = 240;
            globalBPM = static_cast<float>(bpm);
            return true;
        }
        
        // Pack selection
        if (address == "/simpleosc/pack/select") {
            selectedPack = (int)value;
            return true;
        }
        
        // Global grain controls
        if (address == "/simpleosc/grain/size") {
            grainSize = value; // Expecting ms directly
            return true;
        }
        if (address == "/simpleosc/grain/overlap") {
            grainOverlap = value; // Expecting 0-1 directly
            return true;
        }
        
        // Mix mode and level
        if (address == "/simpleosc/mix/mode") {
            int mode = (int)value;
            if (mode >= 0 && mode <= 2) {
                mixMode = static_cast<MixMode>(mode);
            }
            return true;
        }
        if (address == "/simpleosc/mix/level") {
            mixLevel = value;
            return true;
        }
        
        // Track-specific controls using regex
        std::regex trackPattern("/simpleosc/track/([1-8])/(.+)");
        std::smatch matches;
        
        if (std::regex_match(address, matches, trackPattern)) {
            int trackNum = std::stoi(matches[1].str()) - 1; // Convert to 0-based index
            if (trackNum >= 0 && trackNum < 8) {
                std::string param = matches[2].str();
                
                if (param == "select") {
                    selectedTracks[trackNum] = (int)value;
                    return true;
                }
                else if (param == "trigger") {
                    trackControls[trackNum].trigger = (value > 0.0f);
                    return true;
                }
                else if (param == "pitch") {
                    trackControls[trackNum].pitch = OSCScaling::scalePitch(value);
                    return true;
                }
                else if (param == "volume") {
                    trackControls[trackNum].volume = OSCScaling::scaleVolume(value);
                    return true;
                }
                else if (param == "pan") {
                    trackControls[trackNum].pan = OSCScaling::scalePan(value);
                    return true;
                }
                else if (param == "cue/select") {
                    trackControls[trackNum].cuePoint = (int)value;
                    return true;
                }
                else if (param == "cue/trigger") {
                    trackControls[trackNum].cueTrigger = (value > 0.0f);
                    return true;
                }
                else if (param == "loop/mode") {
                    trackControls[trackNum].loopMode = (int)value;
                    return true;
                }
                else if (param == "loop/count") {
                    trackControls[trackNum].loopCount = (int)value;
                    return true;
                }
                else if (param == "mix") {
                    mixLevels[trackNum] = value;
                    return true;
                }
            }
        }
        
        // Per-track grain controls
        std::regex grainTrackPattern("/simpleosc/grain/track/([1-8])/(.+)");
        if (std::regex_match(address, matches, grainTrackPattern)) {
            int trackNum = std::stoi(matches[1].str()) - 1;
            if (trackNum >= 0 && trackNum < 8) {
                std::string param = matches[2].str();
                
                if (param == "size") {
                    trackGrainSize[trackNum] = value;
                    return true;
                }
                else if (param == "overlap") {
                    trackGrainOverlap[trackNum] = value;
                    return true;
                }
            }
        }
        
        return false; // Message not handled
    }
    
    // Calculate mix level for a specific track based on mix mode
    float getTrackMixLevel(int trackIndex) {
        switch (mixMode) {
            case MixMode::SELECT: {
                // Discrete track selection based on mixLevel
                int selectedTrack = (int)(mixLevel * 7.999f); // 0-1 maps to 0-7
                return (trackIndex == selectedTrack) ? 1.0f : 0.0f;
            }
            
            case MixMode::CROSSFADE: {
                // Continuous crossfading between adjacent tracks
                float trackPosition = mixLevel * 7.0f; // 0-1 maps to 0-7
                int lowerTrack = (int)floor(trackPosition);
                int upperTrack = lowerTrack + 1;
                float blend = trackPosition - lowerTrack;
                
                if (trackIndex == lowerTrack) {
                    return 1.0f - blend;
                } else if (trackIndex == upperTrack && upperTrack < 8) {
                    return blend;
                } else {
                    return 0.0f;
                }
            }
            
            case MixMode::MULTI_SELECT:
            default:
                return mixLevels[trackIndex];
        }
    }
    
    // Get effective grain size for a track (track-specific or global)
    float getEffectiveGrainSize(int trackIndex) {
        if (trackGrainSize[trackIndex] > 0.0f) {
            return trackGrainSize[trackIndex];
        }
        return grainSize;
    }
    
    // Get effective grain overlap for a track (track-specific or global)
    float getEffectiveGrainOverlap(int trackIndex) {
        if (trackGrainOverlap[trackIndex] > 0.0f) {
            return trackGrainOverlap[trackIndex];
        }
        return grainOverlap;
    }
};
