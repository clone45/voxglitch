#pragma once

#include <thread>
#include <atomic>
#include <memory>
#include <string>
#include <algorithm>
#include <cctype>
#include <regex>

#include <network.hpp>
#include <system.hpp>
#include <random.hpp>

#include "AudioLoader.hpp"

class AsyncSampleLoader {
private:
    std::atomic<bool> loadRequested{false};
    std::atomic<bool> loadComplete{false};
    std::atomic<bool> shouldAbort{false};

    std::unique_ptr<Sample> loadingSample;

    std::unique_ptr<Sample> readySample;

    std::string loadingPath;
    float loadingBPM = 120.0f;

    std::thread loaderThread;

    static const std::regex& googleDriveRegex() {
        static const std::regex regexPattern(R"((https://drive\.google\.com)/file/d/([^/]+)/view.*)");
        return regexPattern;
    }

    static bool isHttpUrl(const std::string& path) {
        return path.rfind("http://", 0) == 0 || path.rfind("https://", 0) == 0;
    }

    static std::string transformDownloadUrl(const std::string& source) {
        std::smatch match;
        if (std::regex_match(source, match, googleDriveRegex())) {
            std::string base = match[1].str();
            std::string id = match[2].str();
            return base + "/uc?export=download&id=" + id;
        }

        return source;
    }

    static std::string getFilenameFromSource(const std::string& source) {
        if (isHttpUrl(source)) {
            std::string urlPath = rack::network::urlPath(source);
            if (!urlPath.empty()) {
                return rack::system::getFilename(urlPath);
            }
        }
        return rack::system::getFilename(source);
    }

    static std::string deriveDisplayName(const std::string& source) {
        std::string filename = getFilenameFromSource(source);
        if (filename.empty()) {
            return source;
        }

        if (isHttpUrl(source)) {
            std::smatch match;
            if (std::regex_match(source, match, googleDriveRegex())) {
                std::string id = match[2].str();
                if (!id.empty()) {
                    return id;
                }
            }
        }

        // Keep the full filename including extension
        return filename;
    }

    static std::string createTempFilePath() {
        std::string tempDir = rack::system::getTempDirectory();
        uint64_t timestampMs = static_cast<uint64_t>(rack::system::getUnixTime() * 1000.0);
        uint32_t randomId = rack::random::u32();
        std::string filename = std::string("Netrunner_") + std::to_string(timestampMs) + "_" + std::to_string(randomId) + ".wav";
        return rack::system::join(tempDir, filename);
    }

public:
    AsyncSampleLoader() = default;

    AsyncSampleLoader(const AsyncSampleLoader&) = delete;
    AsyncSampleLoader& operator=(const AsyncSampleLoader&) = delete;

    AsyncSampleLoader(AsyncSampleLoader&& other) noexcept
        : loadRequested(other.loadRequested.load()),
          loadComplete(other.loadComplete.load()),
          shouldAbort(other.shouldAbort.load()),
          loadingSample(std::move(other.loadingSample)),
          readySample(std::move(other.readySample)),
          loadingPath(std::move(other.loadingPath)),
          loadingBPM(other.loadingBPM),
          loaderThread(std::move(other.loaderThread)) {
    }

    AsyncSampleLoader& operator=(AsyncSampleLoader&& other) noexcept {
        if (this != &other) {
            shouldAbort = true;
            if (loaderThread.joinable()) {
                loaderThread.join();
            }

            loadRequested.store(other.loadRequested.load());
            loadComplete.store(other.loadComplete.load());
            shouldAbort.store(other.shouldAbort.load());
            loadingSample = std::move(other.loadingSample);
            readySample = std::move(other.readySample);
            loadingPath = std::move(other.loadingPath);
            loadingBPM = other.loadingBPM;
            loaderThread = std::move(other.loaderThread);
        }
        return *this;
    }

    ~AsyncSampleLoader() {
        shouldAbort = true;
        if (loaderThread.joinable()) {
            loaderThread.join();
        }
    }

    bool startLoad(const std::string& path, float bpm) {
        if (loadRequested.load()) {
            return false;
        }

        loadingPath = path;
        loadingBPM = bpm;

        loadComplete = false;
        loadRequested = true;

        if (loaderThread.joinable()) {
            loaderThread.join();
        }

        loaderThread = std::thread([this, path]() {
            std::unique_ptr<Sample> newSample(new Sample());

            std::string localPath = path;
            std::string downloadedPath;
            bool downloaded = false;

            auto cleanupDownload = [&]() {
                if (downloaded && !downloadedPath.empty() && rack::system::exists(downloadedPath)) {
                    rack::system::remove(downloadedPath);
                }
            };

            bool remoteSource = isHttpUrl(path);
            if (remoteSource) {
                std::string downloadUrl = transformDownloadUrl(path);
                downloadedPath = createTempFilePath();
                if (!rack::network::requestDownload(downloadUrl, downloadedPath)) {
                    WARN("Netrunner: download failed for %s", path.c_str());
                    cleanupDownload();
                    loadRequested = false;
                    return;
                }

                if (shouldAbort.load()) {
                    cleanupDownload();
                    loadRequested = false;
                    return;
                }

                downloaded = true;
                localPath = downloadedPath;
            }

            // Use FFmpeg AudioLoader to load the audio file
            vgLib_v2::AudioLoader loader;
            vgLib_v2::AudioData audioData;
            std::string error;

            bool success = loader.loadFile(localPath, audioData, error);

            if (shouldAbort.load()) {
                cleanupDownload();
                loadRequested = false;
                return;
            }

            if (success) {
                // Convert AudioData (interleaved) to Sample format (separate L/R buffers)
                newSample->sample_audio_buffer.clear();
                newSample->sample_rate = audioData.sampleRate;
                newSample->channels = audioData.channels;

                if (audioData.channels == 1) {
                    // Mono: duplicate to both channels
                    for (int64_t i = 0; i < audioData.totalSamples; i++) {
                        float sample = audioData.samples[i];
                        newSample->sample_audio_buffer.push_back(sample, sample);
                    }
                } else if (audioData.channels == 2) {
                    // Stereo: de-interleave
                    for (int64_t i = 0; i < audioData.totalSamples; i++) {
                        float left = audioData.samples[i * 2];
                        float right = audioData.samples[i * 2 + 1];
                        newSample->sample_audio_buffer.push_back(left, right);
                    }
                } else {
                    // Multi-channel: use first two channels
                    for (int64_t i = 0; i < audioData.totalSamples; i++) {
                        float left = audioData.samples[i * audioData.channels];
                        float right = audioData.samples[i * audioData.channels + 1];
                        newSample->sample_audio_buffer.push_back(left, right);
                    }
                }

                newSample->sample_length = newSample->sample_audio_buffer.size();
                newSample->loaded = true;

                if (remoteSource) {
                    std::string filename = getFilenameFromSource(path);
                    if (!filename.empty()) {
                        newSample->filename = filename;
                        newSample->display_name = deriveDisplayName(path);
                    }
                    newSample->path = path;
                } else {
                    newSample->filename = rack::system::getFilename(localPath);
                    newSample->display_name = newSample->filename;
                    // Remove file extension from display name
                    size_t lastDot = newSample->display_name.find_last_of('.');
                    if (lastDot != std::string::npos) {
                        newSample->display_name = newSample->display_name.substr(0, lastDot);
                    }
                    newSample->path = localPath;
                }

                readySample = std::move(newSample);
                loadComplete = true;
            } else {
                WARN("Netrunner: failed to load sample from %s - %s", localPath.c_str(), error.c_str());
                if (downloaded) {
                    downloaded = false;
                }
            }

            cleanupDownload();
            loadRequested = false;
        });

        loaderThread.detach();

        return true;
    }

    bool isReady() const {
        return loadComplete.load();
    }

    bool isLoading() const {
        return loadRequested.load();
    }

    std::unique_ptr<Sample> getLoadedSample() {
        if (!loadComplete.load()) {
            return nullptr;
        }

        loadComplete = false;

        return std::move(readySample);
    }

    std::string getLoadingPath() const {
        return loadingPath;
    }

    float getLoadingBPM() const {
        return loadingBPM;
    }

    void cancel() {
        shouldAbort = true;
        if (loaderThread.joinable()) {
            loaderThread.join();
        }
        shouldAbort = false;
        loadRequested = false;
        loadComplete = false;
        readySample.reset();
    }

    void reset() {
        cancel();
        loadingPath.clear();
        loadingBPM = 120.0f;
    }
};