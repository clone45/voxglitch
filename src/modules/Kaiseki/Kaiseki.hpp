#pragma once

// Include vgLib for sample handling
#include "../../vgLib-2.0/sample.hpp"
#include <algorithm>
#include <cmath>

// Include Kaiseki components
#include "KaisekiSamplePlayer.hpp"
#include "DynamicTrack.hpp"
#include "MasterClock.hpp"
#include "KaisekiTimeStretch.hpp"
#include "KaisekiReceiver.hpp"
#include "OSCScaling.hpp"
#include "DynamicOSCMessageHandler.hpp"
#include "TrackFadeManager.hpp"

struct Kaiseki : VoxglitchSamplerModule
{
    static const int NUM_TRACKS = 8;

    enum ParamIds {
        NUM_PARAMS
    };

    enum InputIds {
        RESET_INPUT,
        START_INPUT,
        STOP_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        ENUMS(TRACK_OUTPUT_LEFT, NUM_TRACKS),
        ENUMS(TRACK_OUTPUT_RIGHT, NUM_TRACKS),
        MIX_LEFT_OUTPUT,
        MIX_RIGHT_OUTPUT,
        POLY_LEFT_OUTPUT,
        POLY_RIGHT_OUTPUT,
        CLOCK_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        OSC_STATUS_LIGHT,
        NUM_LIGHTS
    };

    // Cue target information for pending jumps
    struct CueTarget {
        unsigned int position = 0;
        unsigned int loopEnd = 0;
        float normalizedOffset = 0.0f;
    };

    static constexpr float CUE_TRANSITION_TIME = 0.01f; // 10ms fade per cue jump

    // OSC components
    KaisekiReceiver oscReceiver;
    DynamicOSCMessageHandler messageHandler;

    // Dynamic tracks
    DynamicTrack tracks[NUM_TRACKS];
    KaisekiTimeStretch timeStretchers[NUM_TRACKS];
    TrackFadeManager fadeManagers[NUM_TRACKS];

    // Master clock for synchronization
    MasterClock masterClock;

    // Clock sync trigger output
    dsp::PulseGenerator clockPulse;

    // Control input triggers
    dsp::SchmittTrigger resetTrigger;
    dsp::SchmittTrigger startTrigger;
    dsp::SchmittTrigger stopTrigger;

    // Track state
    bool trackTriggered[NUM_TRACKS] = {false};
    bool trackCueTriggered[NUM_TRACKS] = {false};
    float trackActivityBrightness[NUM_TRACKS] = {0.0f};

    // Pending cue transitions (used for click-free position jumps)
    bool pendingCueChange[NUM_TRACKS] = {false};
    unsigned int pendingCuePosition[NUM_TRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int pendingLoopEnd[NUM_TRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
    float pendingCueOffset[NUM_TRACKS] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    // Pitch smoothing to prevent clicks
    float currentPitch[NUM_TRACKS] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    float targetPitch[NUM_TRACKS] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

    // Display information
    std::string lastAddress = "";
    float lastReceivedValue = 0.0f;
    std::string lastStringValue = "";

    // Frame counter to limit polling frequency
    int frameCounter = 0;
    static constexpr int FRAMES_PER_POLL = 64;  // Poll every ~1.3ms at 48kHz

    // Public accessors for widget
    bool oscEnabled() const { return oscReceiver.isEnabled(); }
    int oscPort() const { return oscReceiver.getPort(); }
    int messagesReceived() const { return oscReceiver.getMessagesReceived(); }
    int messagesDropped() const { return oscReceiver.getMessagesDropped(); }

    void setOscPort(int port) { oscReceiver.setPort(port); }
    void startOSC() { oscReceiver.start(); }
    void stopOSC() { oscReceiver.stop(); }
    void resetCounters() {
        oscReceiver.resetCounters();
    }

    // Constructor
    Kaiseki() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Configure track outputs
        for (int i = 0; i < NUM_TRACKS; i++) {
            configOutput(TRACK_OUTPUT_LEFT + i, "Track " + std::to_string(i + 1) + " Left");
            configOutput(TRACK_OUTPUT_RIGHT + i, "Track " + std::to_string(i + 1) + " Right");
        }

        // Configure mix outputs
        configOutput(MIX_LEFT_OUTPUT, "Mix Left");
        configOutput(MIX_RIGHT_OUTPUT, "Mix Right");

        // Configure polyphonic outputs
        configOutput(POLY_LEFT_OUTPUT, "Polyphonic Left");
        configOutput(POLY_RIGHT_OUTPUT, "Polyphonic Right");

        // Configure clock output
        configOutput(CLOCK_OUTPUT, "Clock Sync");

        // Configure control inputs
        configInput(RESET_INPUT, "Reset");
        configInput(START_INPUT, "Start");
        configInput(STOP_INPUT, "Stop");

        // Configure lights
        configLight(OSC_STATUS_LIGHT, "OSC Status");

        // Set up OSC message callback
        oscReceiver.setMessageCallback([this](const OSCMessage& msg) {
            handleOSCMessage(msg);
        });

        // Start OSC receiver automatically
        oscReceiver.start();
    }

    ~Kaiseki() {
        oscReceiver.stop();
    }

    void handleOSCMessage(const OSCMessage& msg) {
        // Store for display
        lastAddress = msg.address;

        bool handled = false;

        // Check if this is a string message (for file paths)
        if (!msg.stringValue.empty()) {
            lastStringValue = msg.stringValue;
            handled = messageHandler.handleStringMessage(msg.address, msg.stringValue);
        }
        // Check for parameterless messages (like triggers)
        else if (!msg.hasParameters) {
            handled = messageHandler.handleTrigger(msg.address);
        }
        // Handle numeric messages
        else {
            lastReceivedValue = msg.value;
            lastStringValue = "";
            handled = messageHandler.handleMessage(msg.address, msg.value);
        }

        // Check for triggers that need immediate action (regardless of message type)
        if (handled) {
            for (int i = 0; i < NUM_TRACKS; i++) {
                // Handle track triggers
                if (messageHandler.trackControls[i].trigger) {
                    trackTriggered[i] = true;
                    messageHandler.trackControls[i].trigger = false;
                    trackActivityBrightness[i] = 1.0f;
                }

                // Handle cue triggers
                if (messageHandler.trackControls[i].cueTrigger) {
                    trackCueTriggered[i] = true;
                    messageHandler.trackControls[i].cueTrigger = false;
                    trackActivityBrightness[i] = 1.0f;
                }
            }
        }
    }

    void process(const ProcessArgs &args) override {
        // Poll for OSC messages
        frameCounter++;
        if (frameCounter >= FRAMES_PER_POLL) {
            frameCounter = 0;
            oscReceiver.poll();
        }

        // Handle control inputs (CV triggers)
        if (inputs[RESET_INPUT].isConnected()) {
            if (resetTrigger.process(inputs[RESET_INPUT].getVoltage())) {
                masterClock.reset();
                for (int i = 0; i < NUM_TRACKS; i++) {
                    if (tracks[i].isLoaded()) {
                        tracks[i].cueForSync();
                    }
                }
            }
        }

        if (inputs[START_INPUT].isConnected()) {
            if (startTrigger.process(inputs[START_INPUT].getVoltage())) {
                masterClock.start();
            }
        }

        if (inputs[STOP_INPUT].isConnected()) {
            if (stopTrigger.process(inputs[STOP_INPUT].getVoltage())) {
                masterClock.stop();
            }
        }

        // Handle master commands from OSC
        if (messageHandler.checkMasterStop()) {
            masterClock.stop();
        }

        if (messageHandler.checkMasterRestart()) {
            masterClock.reset();
            for (int i = 0; i < NUM_TRACKS; i++) {
                if (tracks[i].isLoaded()) {
                    tracks[i].cueForSync();
                }
            }
        }

        if (messageHandler.checkMasterStart()) {
            masterClock.start();
        }

        // Handle BPM changes from OSC
        float newBPM;
        if (messageHandler.checkBPMChange(newBPM)) {
            // DEBUG("OSC BPM change: %.1f", newBPM);
            masterClock.setMasterBPM(newBPM);
            // If clock isn't active yet, activate it
            if (!masterClock.getIsActive()) {
                masterClock.initFromFirstSample(newBPM, args.sampleRate);
                // DEBUG("Master clock activated with OSC BPM: %.1f", newBPM);
            }
        }

        // Handle track load/unload requests
        for (int i = 0; i < NUM_TRACKS; i++) {
            std::string loadPath;
            if (messageHandler.checkLoadRequest(i, loadPath)) {
                // Check if this is the same file that's already loaded or being loaded
                if (tracks[i].isSameFile(loadPath)) {
                    // DEBUG("Ignoring redundant load request for track %d: %s (already loaded)", i + 1, loadPath.c_str());
                    continue;  // Skip this load request
                }

                // DEBUG("Processing async load request for track %d: %s", i + 1, loadPath.c_str());

                // Start fade-out if track is already playing, otherwise start loading immediately
                if (tracks[i].isLoaded() && tracks[i].isPlaying()) {
                    // Start fade-out before async loading
                    fadeManagers[i].startFadeOut(loadPath, args.sampleRate);
                } else {
                    // Start async loading immediately if track not playing
                    tracks[i].startAsyncLoad(loadPath);
                }
            }

            // Check if fade-out is complete and ready to start async load
            if (fadeManagers[i].readyToLoad()) {
                std::string pathToLoad = fadeManagers[i].getPendingLoadPath();
                // DEBUG("Fade-out complete, starting async load for track %d: %s", i + 1, pathToLoad.c_str());
                tracks[i].startAsyncLoad(pathToLoad);
                fadeManagers[i].markLoadComplete();
            }

            // Check if async loading is complete
            if (tracks[i].checkAsyncLoadComplete()) {
                // DEBUG("Async load complete for track %d", i + 1);

                // Handle first sample initialization
                if (!masterClock.getIsActive()) {
                    float bpm = tracks[i].getBPM();
                    // DEBUG("First sample loaded - initializing master clock with BPM %.1f", bpm);
                    masterClock.initFromFirstSample(bpm, args.sampleRate);
                    tracks[i].startPlaying(); // First track plays immediately
                    // DEBUG("Track %d started playing immediately (first track)", i + 1);
                } else {
                    // DEBUG("Track %d loaded and CUED (waiting for sync), Master BPM: %.1f", i + 1, masterClock.getMasterBPM());
                }

                trackActivityBrightness[i] = 1.0f;
            }

            if (messageHandler.checkUnloadRequest(i)) {
                // DEBUG("Processing unload request for track %d", i + 1);
                tracks[i].clear();
                timeStretchers[i].reset();
                fadeManagers[i].reset();
                pendingCueChange[i] = false;
            }
        }

        // Step master clock and check for sync point
        bool syncPoint = masterClock.step();
        if (syncPoint) {
            // Trigger clock output pulse
            clockPulse.trigger(0.01f);

            // Start all cued tracks
            for (int i = 0; i < NUM_TRACKS; i++) {
                if (tracks[i].isCued()) {
                    tracks[i].startPlaying();
                    trackActivityBrightness[i] = 1.0f;
                }
            }
        }

        // Initialize mix outputs
        float mixLeftSum = 0.0f;
        float mixRightSum = 0.0f;

        // Process each track
        for (int i = 0; i < NUM_TRACKS; i++) {
            if (tracks[i].isLoaded() && tracks[i].isPlaying()) {
                KaisekiSamplePlayer& player = tracks[i].getSamplePlayer();

                // Handle manual triggers (force immediate play)
                if (trackTriggered[i]) {
                    player.trigger(0.0, false);
                    tracks[i].startPlaying(); // Force to playing state
                    trackTriggered[i] = false;
                }

                // Apply deferred cue changes once fade-out reaches silence
                if (pendingCueChange[i] && fadeManagers[i].readyForManualReposition()) {
                    applyCueTarget(i, pendingCuePosition[i], pendingLoopEnd[i], pendingCueOffset[i]);
                    fadeManagers[i].startManualFadeIn(args.sampleRate, CUE_TRANSITION_TIME);
                    pendingCueChange[i] = false;
                }

                // Handle cue triggers - start a short fade and schedule the position jump
                if (trackCueTriggered[i]) {
                    CueTarget target = computeCueTarget(i);

                    bool scheduled = fadeManagers[i].startManualTransition(args.sampleRate, CUE_TRANSITION_TIME);
                    if (scheduled) {
                        pendingCueChange[i] = true;
                        pendingCuePosition[i] = target.position;
                        pendingLoopEnd[i] = target.loopEnd;
                        pendingCueOffset[i] = target.normalizedOffset;
                    } else {
                        // Fallback: apply immediately if fade manager busy (e.g. during load)
                        applyCueTarget(i, target.position, target.loopEnd, target.normalizedOffset);
                    }

                    trackCueTriggered[i] = false;
                }

                // Update loop mode
                int loopMode = messageHandler.trackControls[i].loopMode;
                int repeatCount = -1; // Default infinite
                if (loopMode == 0) repeatCount = 0; // One-shot
                else if (loopMode == 1) repeatCount = messageHandler.trackControls[i].loopCount; // Finite
                player.setRepeatCount(repeatCount);

                // Make sure sample is playing
                if (!player.playing) {
                    player.trigger(0.0, false);
                }

                // Step the sample player at neutral rate (matching Tracks strategy)
                player.step(0.0, 0.0, 1.0, true);

                // Calculate pitch multiplier from OSC control
                float pitchValue = messageHandler.trackControls[i].pitch;

                // Add a small deadzone around neutral pitch to ensure exactly 1.0 when centered
                float pitchMultiplier = 1.0f;
                if (std::abs(pitchValue) > 0.01f) {  // Only apply pitch if significantly off-center
                    pitchMultiplier = std::pow(2.0f, pitchValue);
                    pitchMultiplier = rack::math::clamp(pitchMultiplier, 0.25f, 4.0f);
                }

                // // DEBUG: Track pitch changes
                if (std::abs(currentPitch[i] - pitchMultiplier) > 0.001f) {
                    // DEBUG("Track %d pitch change: %.4f -> %.4f", i + 1, currentPitch[i], pitchMultiplier);
                    currentPitch[i] = pitchMultiplier;
                }

                // Configure time stretcher (matching Tracks order)
                if (masterClock.getIsActive()) {
                    // Sync to master BPM
                    timeStretchers[i].setSourceBPM(tracks[i].getBPM());
                    timeStretchers[i].setTargetBPM(masterClock.getMasterBPM());
                } else {
                    // No stretching if no master clock
                    timeStretchers[i].setSourceBPM(tracks[i].getBPM());
                    timeStretchers[i].setTargetBPM(tracks[i].getBPM());
                }
                timeStretchers[i].setGrainSize(messageHandler.getEffectiveGrainSize(i));
                timeStretchers[i].setOverlapRatio(messageHandler.getEffectiveGrainOverlap(i));
                timeStretchers[i].setPitchMultiplier(pitchMultiplier);

                // Second step with pitch applied (matching Tracks strategy)
                player.step(pitchMultiplier - 1.0f, 0.0, 1.0, true);

                // Get time-stretched audio
                std::pair<float, float> stretchedAudio = timeStretchers[i].process(player);
                float leftSample = stretchedAudio.first;
                float rightSample = stretchedAudio.second;

                // Apply volume
                float volume = messageHandler.trackControls[i].volume;
                leftSample *= volume;
                rightSample *= volume;

                // Apply fade manager to track outputs (for smooth sample loading transitions)
                fadeManagers[i].process(&leftSample, &rightSample);

                // Output raw audio to individual track outputs
                outputs[TRACK_OUTPUT_LEFT + i].setVoltage(leftSample * 5.0f);
                outputs[TRACK_OUTPUT_RIGHT + i].setVoltage(rightSample * 5.0f);

                // Output to polyphonic outputs
                outputs[POLY_LEFT_OUTPUT].setVoltage(leftSample * 5.0f, i);
                outputs[POLY_RIGHT_OUTPUT].setVoltage(rightSample * 5.0f, i);

                // Apply mix level
                float mixLevel = messageHandler.mixLevels[i];
                float leftSampleMixed = leftSample * mixLevel;
                float rightSampleMixed = rightSample * mixLevel;

                // Apply panning
                float panPosition = messageHandler.trackControls[i].pan;
                float leftGain, rightGain;
                if (panPosition <= 0.0f) {
                    leftGain = 1.0f;
                    rightGain = 1.0f + panPosition;
                } else {
                    leftGain = 1.0f - panPosition;
                    rightGain = 1.0f;
                }

                float leftSamplePanned = leftSampleMixed * leftGain;
                float rightSamplePanned = rightSampleMixed * rightGain;

                // Add to mix outputs
                mixLeftSum += leftSamplePanned;
                mixRightSum += rightSamplePanned;
            } else {
                // Clear outputs for inactive tracks
                outputs[TRACK_OUTPUT_LEFT + i].setVoltage(0.0f);
                outputs[TRACK_OUTPUT_RIGHT + i].setVoltage(0.0f);
                outputs[POLY_LEFT_OUTPUT].setVoltage(0.0f, i);
                outputs[POLY_RIGHT_OUTPUT].setVoltage(0.0f, i);
            }

            // Update activity lights
            if (trackActivityBrightness[i] > 0.0f) {
                trackActivityBrightness[i] -= args.sampleTime * 5.0f;
                if (trackActivityBrightness[i] < 0.0f) trackActivityBrightness[i] = 0.0f;
            }
        }

        // Set polyphonic channel counts
        outputs[POLY_LEFT_OUTPUT].setChannels(NUM_TRACKS);
        outputs[POLY_RIGHT_OUTPUT].setChannels(NUM_TRACKS);

        // Output final mix signals
        outputs[MIX_LEFT_OUTPUT].setVoltage(mixLeftSum * 5.0f);
        outputs[MIX_RIGHT_OUTPUT].setVoltage(mixRightSum * 5.0f);

        // Process and output clock pulse
        bool clockTrig = clockPulse.process(1.0 / args.sampleRate);
        outputs[CLOCK_OUTPUT].setVoltage(clockTrig ? 10.0f : 0.0f);

        // Update status light
        lights[OSC_STATUS_LIGHT].setBrightness(1.0f);
    }


    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        masterClock.updateSampleRate(e.sampleRate);
        for (int i = 0; i < NUM_TRACKS; i++) {
            if (tracks[i].isLoaded()) {
                tracks[i].getSamplePlayer().updateSampleRate();
            }
            timeStretchers[i].reset();
            fadeManagers[i].reset();
        }
    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();
        json_object_set_new(rootJ, "oscPort", json_integer(oscReceiver.getPort()));
        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t* portJ = json_object_get(rootJ, "oscPort");
        if (portJ) {
            oscReceiver.setPort(json_integer_value(portJ));
        }
    }

private:
    CueTarget computeCueTarget(int trackIndex) {
        CueTarget target;

        DynamicTrack& track = tracks[trackIndex];
        unsigned int sampleSize = track.getSampleSize();
        if (sampleSize == 0) {
            return target;
        }

        auto& cuePoints = track.getCuePoints();
        if (!cuePoints.empty()) {
            int cueIndex = messageHandler.trackControls[trackIndex].cuePoint;
            cueIndex = rack::math::clamp(cueIndex, 0, (int)cuePoints.size() - 1);
            target.position = cuePoints[cueIndex];
            if (cueIndex + 1 < (int)cuePoints.size()) {
                target.loopEnd = cuePoints[cueIndex + 1];
            }

            if (sampleSize > 1) {
                float denom = static_cast<float>(sampleSize - 1);
                target.normalizedOffset = rack::math::clamp(static_cast<float>(target.position) / denom, 0.0f, 1.0f);
            } else {
                target.normalizedOffset = 0.0f;
            }
        } else {
            float normalized = rack::math::clamp(messageHandler.trackControls[trackIndex].cueOffset, 0.0f, 1.0f);
            target.normalizedOffset = normalized;
            unsigned int maxIndex = (sampleSize > 0) ? (sampleSize - 1) : 0;
            target.position = (unsigned int)std::round(normalized * maxIndex);
        }

        if (target.loopEnd > sampleSize) {
            target.loopEnd = sampleSize;
        }

        return target;
    }

    void applyCueTarget(int trackIndex, unsigned int cuePosition, unsigned int loopEnd, float normalizedOffset) {
        DynamicTrack& track = tracks[trackIndex];
        KaisekiSamplePlayer& player = track.getSamplePlayer();

        unsigned int sampleSize = track.getSampleSize();
        if (sampleSize == 0) {
            return;
        }

        cuePosition = std::min(cuePosition, sampleSize - 1);
        
        if (loopEnd > cuePosition) {
            loopEnd = std::min(loopEnd, sampleSize);
            player.setLoopBoundaries(cuePosition, loopEnd);
        } else {
            player.setLoopBoundaries(cuePosition, 0);
        }

        player.setPosition(cuePosition);
        timeStretchers[trackIndex].setOffset(normalizedOffset);
        timeStretchers[trackIndex].setPosition(static_cast<float>(cuePosition));

        if (!player.playing) {
            player.playing = true;
        }
    }
};
