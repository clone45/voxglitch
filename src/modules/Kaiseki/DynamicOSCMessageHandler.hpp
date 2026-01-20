#pragma once

#include <string>
#include <regex>
#include <cmath>
#include "OSCScaling.hpp"
#include "../../vgLib-2.0/helpers/Debugging.hpp"

// OSC message handler for dynamic sample loading
class DynamicOSCMessageHandler {
public:
    struct TrackControl {
        float pitch = 0.0f;
        float volume = 1.0f;
        float pan = 0.0f;
        int cuePoint = 0;
        float cueOffset = 0.0f;
        int loopMode = 2; // 0=one-shot, 1=finite, 2=infinite
        int loopCount = 1;
        bool trigger = false;
        bool cueTrigger = false;
        std::string loadPath = "";     // Path for sample to load
        bool loadRequested = false;    // Flag for load request
        bool unloadRequested = false;  // Flag for unload request
    };

    // Global controls
    float globalBPM = 120.0f;
    float grainSize = 175.0f; // ms
    float grainOverlap = 0.75f;  // Match time stretcher default!
    float mixLevels[8] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

    // Per-track controls
    TrackControl trackControls[8];

    // Per-track grain controls (optional)
    float trackGrainSize[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // 0 = use global
    float trackGrainOverlap[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // 0 = use global

    // Master control
    bool masterStop = false;
    bool masterRestart = false;
    bool masterStart = false;
    bool bpmChanged = false;

    DynamicOSCMessageHandler() {
        reset();
    }

    void reset() {
        globalBPM = 120.0f;
        grainSize = 175.0f;
        grainOverlap = 0.75f;
        masterStop = false;
        masterRestart = false;
        masterStart = false;
        bpmChanged = false;

        for (int i = 0; i < 8; i++) {
            mixLevels[i] = 1.0f;
            trackControls[i] = TrackControl();
            trackGrainSize[i] = 0.0f;
            trackGrainOverlap[i] = 0.0f;
        }
    }

    // Handle parameterless trigger messages
    bool handleTrigger(const std::string& address) {
        // Track-specific trigger controls using regex
        std::regex trackPattern("/track/([1-8])/(.+)");
        std::smatch matches;

        if (std::regex_match(address, matches, trackPattern)) {
            int trackNum = std::stoi(matches[1].str()) - 1; // Convert to 0-based index
            if (trackNum >= 0 && trackNum < 8) {
                std::string param = matches[2].str();

                if (param == "cue/trigger") {
                    trackControls[trackNum].cueTrigger = true;
                    return true;
                }
                else if (param == "trigger") {
                    trackControls[trackNum].trigger = true;
                    return true;
                }
            }
        }

        return false;
    }

    // Parse and handle OSC message - now handles both float and string values
    bool handleMessage(const std::string& address, float value) {
        // Global BPM control
        if (address == "/bpm") {
            int bpm = static_cast<int>(std::round(value));
            if (bpm < 20) bpm = 20;
            if (bpm > 240) bpm = 240;
            globalBPM = static_cast<float>(bpm);
            bpmChanged = true;
            return true;
        }

        // Global grain controls with proper scaling
        if (address == "/grain/size") {
            // Map 0-1 to 50-300ms range
            grainSize = 50.0f + (value * 250.0f);
            return true;
        }
        if (address == "/grain/overlap") {
            // Map 0-1 to 0.25-0.95 range
            grainOverlap = 0.25f + (value * 0.70f);
            return true;
        }

        // Master controls
        if (address == "/master/stop") {
            masterStop = (value > 0.0f);
            return true;
        }
        if (address == "/master/restart") {
            masterRestart = (value > 0.0f);
            return true;
        }
        if (address == "/master/start") {
            masterStart = (value > 0.0f);
            return true;
        }

        // Track-specific controls using regex
        std::regex trackPattern("/track/([1-8])/(.+)");
        std::smatch matches;

        if (std::regex_match(address, matches, trackPattern)) {
            int trackNum = std::stoi(matches[1].str()) - 1; // Convert to 0-based index
            if (trackNum >= 0 && trackNum < 8) {
                std::string param = matches[2].str();

                if (param == "trigger") {
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
                else if (param == "cue/offset") {
                    VBUG << "Setting track " << (trackNum+1) << " cue offset: "
                         << trackControls[trackNum].cueOffset << " -> " << value;
                    trackControls[trackNum].cueOffset = value;
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
                else if (param == "unload") {
                    trackControls[trackNum].unloadRequested = (value > 0.0f);
                    return true;
                }
                else if (param == "force_trigger") {
                    // Force immediate play, skipping cue
                    trackControls[trackNum].trigger = (value > 0.0f);
                    return true;
                }
            }
        }

        // Per-track grain controls
        std::regex grainTrackPattern("/grain/track/([1-8])/(.+)");
        if (std::regex_match(address, matches, grainTrackPattern)) {
            int trackNum = std::stoi(matches[1].str()) - 1;
            if (trackNum >= 0 && trackNum < 8) {
                std::string param = matches[2].str();

                if (param == "size") {
                    // Map 0-1 to 50-300ms range
                    trackGrainSize[trackNum] = 50.0f + (value * 250.0f);
                    return true;
                }
                else if (param == "overlap") {
                    // Map 0-1 to 0.25-0.95 range
                    trackGrainOverlap[trackNum] = 0.25f + (value * 0.70f);
                    return true;
                }
            }
        }

        return false;
    }

    // Handle string messages (for file paths)
    bool handleStringMessage(const std::string& address, const std::string& value) {
        // Track load commands
        std::regex loadPattern("/track/([1-8])/load");
        std::smatch matches;

        if (std::regex_match(address, matches, loadPattern)) {
            int trackNum = std::stoi(matches[1].str()) - 1;
            if (trackNum >= 0 && trackNum < 8) {
                trackControls[trackNum].loadPath = value;
                trackControls[trackNum].loadRequested = true;
                return true;
            }
        }

        return false;
    }

    // Get effective grain size for a track
    float getEffectiveGrainSize(int trackIndex) {
        if (trackGrainSize[trackIndex] > 0.0f) {
            return trackGrainSize[trackIndex];
        }
        return grainSize;
    }

    // Get effective grain overlap for a track
    float getEffectiveGrainOverlap(int trackIndex) {
        if (trackGrainOverlap[trackIndex] > 0.0f) {
            return trackGrainOverlap[trackIndex];
        }
        return grainOverlap;
    }

    // Check and clear load request
    bool checkLoadRequest(int trackIndex, std::string& pathOut) {
        if (trackControls[trackIndex].loadRequested) {
            pathOut = trackControls[trackIndex].loadPath;
            trackControls[trackIndex].loadRequested = false;
            trackControls[trackIndex].loadPath = "";
            return true;
        }
        return false;
    }

    // Check and clear unload request
    bool checkUnloadRequest(int trackIndex) {
        if (trackControls[trackIndex].unloadRequested) {
            trackControls[trackIndex].unloadRequested = false;
            return true;
        }
        return false;
    }

    // Check and clear master commands
    bool checkMasterStop() {
        if (masterStop) {
            masterStop = false;
            return true;
        }
        return false;
    }

    bool checkMasterRestart() {
        if (masterRestart) {
            masterRestart = false;
            return true;
        }
        return false;
    }

    bool checkMasterStart() {
        if (masterStart) {
            masterStart = false;
            return true;
        }
        return false;
    }

    bool checkBPMChange(float& bpmOut) {
        if (bpmChanged) {
            bpmOut = globalBPM;
            bpmChanged = false;
            return true;
        }
        return false;
    }
};
