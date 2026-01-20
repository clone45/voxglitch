#pragma once

#include <string>
struct TrackFadeManager {
    enum FadeState {
        NORMAL,
        FADING_OUT,
        LOADING,
        FADING_IN,
        MANUAL_WAIT
    };

    FadeState state = NORMAL;
    float fadeValue = 1.0f;
    float fadeRate = 0.0f;

    // Pending load information
    std::string pendingLoadPath;
    float pendingSampleRate = 0.0f;
    bool hasPendingLoad = false;
    bool loadComplete = false;

    // Manual transition state
    bool manualPending = false;

    // Fade duration in seconds
    float fadeOutDuration = 0.05f;  // 50ms fade out
    float fadeInDuration = 0.05f;   // 50ms fade in

    void startFadeOut(const std::string& loadPath, float sampleRate, float fadeOutTime = 0.05f) {
        if (state == NORMAL) {
            state = FADING_OUT;
            fadeValue = 1.0f;
            fadeOutDuration = fadeOutTime;
            fadeRate = 1.0f / (fadeOutDuration * sampleRate);

            // Store the pending load information
            pendingLoadPath = loadPath;
            pendingSampleRate = sampleRate;
            hasPendingLoad = true;
            loadComplete = false;
            manualPending = false;
        }
    }

    void startFadeIn(float sampleRate, float fadeInTime = 0.05f) {
        state = FADING_IN;
        fadeValue = 0.0f;
        fadeInDuration = fadeInTime;
        fadeRate = 1.0f / (fadeInDuration * sampleRate);
    }

    bool startManualTransition(float sampleRate, float fadeOutTime = 0.01f) {
        if (state != NORMAL) {
            return false;
        }

        manualPending = true;
        state = FADING_OUT;
        fadeValue = 1.0f;
        fadeOutDuration = fadeOutTime;
        fadeRate = 1.0f / (fadeOutDuration * sampleRate);
        return true;
    }

    bool readyForManualReposition() const {
        return manualPending && state == MANUAL_WAIT;
    }

    void startManualFadeIn(float sampleRate, float fadeInTime = 0.01f) {
        if (!manualPending || state != MANUAL_WAIT) {
            return;
        }

        state = FADING_IN;
        fadeValue = 0.0f;
        fadeInDuration = fadeInTime;
        fadeRate = 1.0f / (fadeInDuration * sampleRate);
        manualPending = false;
    }

    bool readyToLoad() {
        return state == LOADING && hasPendingLoad;
    }

    void markLoadComplete() {
        loadComplete = true;
        hasPendingLoad = false;
    }

    void clearPendingLoad() {
        hasPendingLoad = false;
        pendingLoadPath.clear();
    }

    std::string getPendingLoadPath() {
        return pendingLoadPath;
    }

    void process(float* leftAudio, float* rightAudio) {
        // Early exit for normal state - most common case
        if (state == NORMAL) {
            return;  // Audio passes through unchanged
        }

        switch (state) {
            case NORMAL:
                // Already handled above
                break;

            case FADING_OUT:
                fadeValue -= fadeRate;
                if (fadeValue <= 0.0f) {
                    fadeValue = 0.0f;
                    if (manualPending) {
                        state = MANUAL_WAIT;
                    } else {
                        state = LOADING;
                    }
                }
                *leftAudio *= fadeValue;
                *rightAudio *= fadeValue;
                break;

            case LOADING:
                // Mute audio while loading
                *leftAudio = 0.0f;
                *rightAudio = 0.0f;

                // If load is complete, start fading in
                if (loadComplete) {
                    startFadeIn(pendingSampleRate);
                    loadComplete = false;
                }
                break;

            case FADING_IN:
                fadeValue += fadeRate;
                if (fadeValue >= 1.0f) {
                    fadeValue = 1.0f;
                    state = NORMAL;
                }
                *leftAudio *= fadeValue;
                *rightAudio *= fadeValue;
                break;

            case MANUAL_WAIT:
                *leftAudio = 0.0f;
                *rightAudio = 0.0f;
                break;
        }
    }

    bool isTransitioning() {
        return state != NORMAL;
    }

    bool isFadingOut() {
        return state == FADING_OUT;
    }

    bool isFadingIn() {
        return state == FADING_IN;
    }

    bool isLoading() {
        return state == LOADING;
    }

    void reset() {
        state = NORMAL;
        fadeValue = 1.0f;
        fadeRate = 0.0f;
        hasPendingLoad = false;
        loadComplete = false;
        pendingLoadPath.clear();
        manualPending = false;
    }

    float getFadeValue() {
        return fadeValue;
    }
};
