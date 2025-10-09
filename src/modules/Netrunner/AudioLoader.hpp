#pragma once

#include <string>
#include <vector>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

namespace vgLib_v2 {

struct AudioData {
    std::vector<float> samples;  // Interleaved float samples
    int sampleRate;
    int channels;
    int64_t totalSamples;  // Total samples per channel

    AudioData() : sampleRate(0), channels(0), totalSamples(0) {}
};

class AudioLoader {
public:
    AudioLoader();
    ~AudioLoader();

    // Main loading function - loads entire file into memory
    bool loadFile(const std::string& filepath, AudioData& outData, std::string& errorMsg);

    // Get file information without loading
    bool getFileInfo(const std::string& filepath, int& sampleRate,
                     int& channels, int64_t& totalSamples, std::string& errorMsg);

private:
    // FFmpeg context structures
    AVFormatContext* formatCtx;
    AVCodecContext* codecCtx;
    AVPacket* packet;
    AVFrame* frame;
    int audioStreamIndex;

    // Internal methods
    bool openFile(const std::string& filepath, std::string& errorMsg);
    bool findAudioStream(std::string& errorMsg);
    bool openCodec(std::string& errorMsg);
    bool decodeAllFrames(AudioData& outData, std::string& errorMsg);
    void cleanup();

    // Sample conversion
    float getSample(const AVCodecContext* ctx, uint8_t* buffer, int sampleIndex);
    void processFrame(const AVFrame* frame, std::vector<float>& outSamples);
};

} // namespace vgLib_v2
