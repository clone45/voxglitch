#pragma once

#include "../../vgLib-2.0/sample.hpp"
#include "KaisekiSamplePlayer.hpp"
#include "AsyncSampleLoader.hpp"
#include <regex>
#include <string>
#include <memory>
#include <algorithm>
#include <cctype>

#include <network.hpp>
#include <system.hpp>

enum TrackState {
    EMPTY,     // No sample loaded
    CUED,      // Sample loaded, waiting for sync point
    PLAYING    // Actively playing
};

class DynamicTrack {
private:
    KaisekiSamplePlayer samplePlayer;
    AsyncSampleLoader asyncLoader;
    TrackState state = EMPTY;
    float originalBPM = 120.0f;
    std::string trackName = "[ EMPTY ]";
    std::string loadedPath = "";  // Track the currently loaded file path
    std::string pendingPath = "";  // Path being loaded asynchronously
    std::vector<unsigned int> cuePoints;

    static bool isHttpUrl(const std::string& path) {
        return path.rfind("http://", 0) == 0 || path.rfind("https://", 0) == 0;
    }

    static std::string getNormalizedPath(const std::string& source) {
        if (isHttpUrl(source)) {
            std::string urlPath = rack::network::urlPath(source);
            if (!urlPath.empty()) {
                return urlPath;
            }
        }
        return source;
    }

    static std::string getFilenameFromSource(const std::string& source) {
        std::string normalized = getNormalizedPath(source);
        return rack::system::getFilename(normalized);
    }

    static std::string deriveDisplayName(const std::string& source) {
        std::string filename = getFilenameFromSource(source);
        if (filename.empty()) {
            return source;
        }

        std::string display = filename;
        size_t dotPos = display.find_last_of('.');
        if (dotPos != std::string::npos) {
            std::string ext = display.substr(dotPos);
            std::string extLower = ext;
            std::transform(extLower.begin(), extLower.end(), extLower.begin(), [](unsigned char c) { return std::tolower(c); });
            if (extLower == ".wav") {
                display = display.substr(0, dotPos);
            }
        }

        return display;
    }

public:
    DynamicTrack() {
        state = EMPTY;
        trackName = "[ EMPTY ]";
        cuePoints.clear();
    }

    ~DynamicTrack() {
        clear();
    }

    void clear() {
        samplePlayer.releaseSample();
        asyncLoader.reset();
        state = EMPTY;
        trackName = "[ EMPTY ]";
        loadedPath = "";
        pendingPath = "";
        originalBPM = 120.0f;
        cuePoints.clear();
    }

    // Start loading a sample asynchronously
    bool startAsyncLoad(const std::string& path) {
        // Extract BPM before starting async load
        float bpm = extractBPMFromFilename(path);

        // Start background loading
        if (asyncLoader.startLoad(path, bpm)) {
            pendingPath = path;
            // DEBUG("Started async load for: %s", path.c_str());
            return true;
        }
        return false;
    }

    // Check if async load is complete and apply it
    bool checkAsyncLoadComplete() {
        if (!asyncLoader.isReady()) {
            return false;
        }

        // Get the loaded sample
        std::unique_ptr<Sample> newSample = asyncLoader.getLoadedSample();
        if (!newSample) {
            // DEBUG("Async load failed - no sample returned");
            return false;
        }

        // Apply the loaded sample to the player
        samplePlayer.applySample(std::move(newSample));

        // Update metadata
        loadedPath = pendingPath;
        originalBPM = asyncLoader.getLoadingBPM();

        // Set track name from filename
        trackName = deriveDisplayName(pendingPath);
        if (trackName.empty()) {
            trackName = "[ EMPTY ]";
        }

        // Sample loaded but not yet playing
        state = CUED;

        // Initialize player
        samplePlayer.setLoopBoundaries(0, 0);
        samplePlayer.trigger(0.0, false);

        samplePlayer.setFilename(getFilenameFromSource(loadedPath));
        samplePlayer.setPath(loadedPath);
        samplePlayer.sample.display_name = trackName;

        pendingPath.clear();
        // DEBUG("Async load complete for: %s", loadedPath.c_str());
        return true;
    }

    // Legacy synchronous load (kept for compatibility but should be avoided)
    bool loadSample(const std::string& path) {
        // Load the sample
        if (!samplePlayer.loadSample(path)) {
            clear();
            return false;
        }

        // Store the loaded path
        loadedPath = path;

        // Extract BPM from filename
        originalBPM = extractBPMFromFilename(path);

        // Set track name from filename
        trackName = deriveDisplayName(path);
        if (trackName.empty()) {
            trackName = "[ EMPTY ]";
        }

        // Sample loaded but not yet playing
        state = CUED;

        // Initialize player
        samplePlayer.setLoopBoundaries(0, 0);
        samplePlayer.trigger(0.0, false);

        samplePlayer.setFilename(getFilenameFromSource(path));
        samplePlayer.setPath(path);
        samplePlayer.sample.display_name = trackName;

        return true;
    }

    void startPlaying() {
        if (state == CUED) {
            state = PLAYING;
            samplePlayer.trigger(0.0, false);
        }
    }

    void cueForSync() {
        if (state == PLAYING && isLoaded()) {
            state = CUED;
            samplePlayer.setPosition(0);
        }
    }

    TrackState getState() const {
        return state;
    }

    bool isLoaded() const {
        return const_cast<KaisekiSamplePlayer&>(samplePlayer).isLoaded();
    }

    bool isCued() const {
        return state == CUED;
    }

    bool isPlaying() const {
        return state == PLAYING;
    }

    KaisekiSamplePlayer& getSamplePlayer() {
        return samplePlayer;
    }

    float getBPM() const {
        return originalBPM;
    }

    const std::string& getName() const {
        return trackName;
    }

    std::vector<unsigned int>& getCuePoints() {
        return cuePoints;
    }

    void setCuePoints(const std::vector<unsigned int>& points) {
        cuePoints = points;
    }

    const std::string& getLoadedPath() const {
        return loadedPath;
    }

    bool isSameFile(const std::string& path) const {
        return (!loadedPath.empty() && loadedPath == path) ||
               (!pendingPath.empty() && pendingPath == path);
    }

    bool isLoadingAsync() const {
        return asyncLoader.isLoading();
    }

    unsigned int getSampleSize() {
        return samplePlayer.sample.size();
    }

private:
    float extractBPMFromFilename(const std::string& path) {
        std::string filename = getFilenameFromSource(path);
        // DEBUG("BPM extraction from filename: %s", filename.c_str());

        // Try various BPM patterns in filename
        // Pattern 1: "kick_120bpm.wav", "loop-140-bpm.wav", "sample_90_BPM.wav"
        std::regex bpm_regex("(\\d+)[_\\-]?bpm", std::regex::icase);
        std::smatch match;

        if (std::regex_search(filename, match, bpm_regex)) {
            try {
                float bpm = std::stof(match[1].str());
                // DEBUG("Pattern 1 match: %s -> %.1f BPM", match[1].str().c_str(), bpm);
                if (bpm > 0 && bpm < 300) { // Sanity check
                    // DEBUG("BPM extraction successful (Pattern 1): %.1f", bpm);
                    return bpm;
                }
                // DEBUG("Pattern 1 BPM out of range: %.1f", bpm);
            } catch (...) {
                // DEBUG("Pattern 1 conversion failed");
            }
        } else {
            // DEBUG("Pattern 1 no match");
        }

        // Pattern 2: Standalone number like "PLX_ELT_128_synth", "095_Gret_Bass", "loop_120_"
        // Look for underscore-delimited 2-3 digit numbers that are likely BPMs
        std::regex standalone_regex("(?:^|_)(\\d{2,3})(?:_|$)");
        if (std::regex_search(filename, match, standalone_regex)) {
            try {
                float bpm = std::stof(match[1].str());
                // DEBUG("Pattern 2 match: %s -> %.1f BPM", match[1].str().c_str(), bpm);
                // Common BPM range check (60-200 is typical for most music)
                // This handles "095" -> 95 BPM
                if (bpm >= 60 && bpm <= 200) {
                    //  DEBUG("BPM extraction successful (Pattern 2): %.1f", bpm);
                    return bpm;
                }
                // DEBUG("Pattern 2 BPM out of range: %.1f", bpm);
            } catch (...) {
                // DEBUG("Pattern 2 conversion failed");
            }
        } else {
            // DEBUG("Pattern 2 no match");
        }

        // Pattern 3: Number at end before extension like "beat120.wav"
        std::regex end_regex("(\\d{2,3})\\.");
        if (std::regex_search(filename, match, end_regex)) {
            try {
                float bpm = std::stof(match[1].str());
                // DEBUG("Pattern 3 match: %s -> %.1f BPM", match[1].str().c_str(), bpm);
                if (bpm >= 60 && bpm <= 200) {
                    // DEBUG("BPM extraction successful (Pattern 3): %.1f", bpm);
                    return bpm;
                }
                // DEBUG("Pattern 3 BPM out of range: %.1f", bpm);
            } catch (...) {
                // DEBUG("Pattern 3 conversion failed");
            }
        } else {
            // DEBUG("Pattern 3 no match");
        }

        // DEBUG("No BPM patterns matched, using default: 120 BPM");
        return 120.0f; // Default BPM
    }
};
