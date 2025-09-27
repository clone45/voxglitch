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

    static std::string createTempFilePath() {
        std::string tempDir = rack::system::getTempDirectory();
        uint64_t timestampMs = static_cast<uint64_t>(rack::system::getUnixTime() * 1000.0);
        uint32_t randomId = rack::random::u32();
        std::string filename = std::string("Netrunner_") + std::to_string(timestampMs) + "_" + std::to_string(randomId) + ".wav";
        return rack::system::join(tempDir, filename);
    }

public:
    AsyncSampleLoader() = default;

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
                INFO("Netrunner: downloading %s to temporary file %s", downloadUrl.c_str(), downloadedPath.c_str());
                if (!rack::network::requestDownload(downloadUrl, downloadedPath)) {
                    WARN("Netrunner: download failed for %s", path.c_str());
                    cleanupDownload();
                    loadRequested = false;
                    return;
                }

                if (shouldAbort.load()) {
                    INFO("Netrunner: download aborted for %s", path.c_str());
                    cleanupDownload();
                    loadRequested = false;
                    return;
                }

                downloaded = true;
                localPath = downloadedPath;
            }

            bool success = newSample->load(localPath);

            if (shouldAbort.load()) {
                INFO("Netrunner: load aborted for %s", path.c_str());
                cleanupDownload();
                loadRequested = false;
                return;
            }

            if (success) {
                if (remoteSource) {
                    std::string filename = getFilenameFromSource(path);
                    if (!filename.empty()) {
                        newSample->filename = filename;
                        newSample->display_name = deriveDisplayName(path);
                    }
                    newSample->path = path;
                }

                readySample = std::move(newSample);
                loadComplete = true;
            } else {
                WARN("Netrunner: failed to load sample from %s", localPath.c_str());
                if (downloaded) {
                    INFO("Netrunner: keeping downloaded file for inspection: %s", localPath.c_str());
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