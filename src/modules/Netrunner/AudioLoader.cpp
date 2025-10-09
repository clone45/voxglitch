#include "AudioLoader.hpp"
#include <cstring>

namespace vgLib_v2 {

AudioLoader::AudioLoader()
    : formatCtx(nullptr)
    , codecCtx(nullptr)
    , packet(nullptr)
    , frame(nullptr)
    , audioStreamIndex(-1)
{
}

AudioLoader::~AudioLoader() {
    cleanup();
}

void AudioLoader::cleanup() {
    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (packet) {
        av_packet_free(&packet);
        packet = nullptr;
    }
    if (codecCtx) {
        avcodec_free_context(&codecCtx);
        codecCtx = nullptr;
    }
    if (formatCtx) {
        avformat_close_input(&formatCtx);
        formatCtx = nullptr;
    }
    audioStreamIndex = -1;
}

bool AudioLoader::loadFile(const std::string& filepath, AudioData& outData, std::string& errorMsg) {
    cleanup();

    // Open file and find streams
    if (!openFile(filepath, errorMsg)) return false;
    if (!findAudioStream(errorMsg)) return false;
    if (!openCodec(errorMsg)) return false;

    // Decode all audio frames
    if (!decodeAllFrames(outData, errorMsg)) return false;

    cleanup();
    return true;
}

bool AudioLoader::openFile(const std::string& filepath, std::string& errorMsg) {
    formatCtx = nullptr;
    int ret = avformat_open_input(&formatCtx, filepath.c_str(), nullptr, nullptr);
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        errorMsg = "Cannot open file: " + std::string(errbuf);
        return false;
    }

    ret = avformat_find_stream_info(formatCtx, nullptr);
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        errorMsg = "Cannot find stream info: " + std::string(errbuf);
        return false;
    }

    return true;
}

bool AudioLoader::findAudioStream(std::string& errorMsg) {
    audioStreamIndex = -1;
    for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1) {
        errorMsg = "No audio stream found in file";
        return false;
    }

    return true;
}

bool AudioLoader::openCodec(std::string& errorMsg) {
    AVCodecParameters* codecPar = formatCtx->streams[audioStreamIndex]->codecpar;

    // Find decoder for this codec
    const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
    if (!codec) {
        errorMsg = "Decoder not found for codec";
        return false;
    }

    // Allocate codec context
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        errorMsg = "Failed to allocate codec context";
        return false;
    }

    // Copy codec parameters to context
    int ret = avcodec_parameters_to_context(codecCtx, codecPar);
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        errorMsg = "Failed to copy codec parameters: " + std::string(errbuf);
        return false;
    }

    // Open codec
    ret = avcodec_open2(codecCtx, codec, nullptr);
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        errorMsg = "Failed to open codec: " + std::string(errbuf);
        return false;
    }

    return true;
}

bool AudioLoader::decodeAllFrames(AudioData& outData, std::string& errorMsg) {
    // Allocate packet and frame
    packet = av_packet_alloc();
    frame = av_frame_alloc();
    if (!packet || !frame) {
        errorMsg = "Failed to allocate packet or frame";
        return false;
    }

    // Store audio properties
    outData.sampleRate = codecCtx->sample_rate;
    outData.channels = codecCtx->ch_layout.nb_channels;
    outData.totalSamples = 0;
    outData.samples.clear();

    // Reserve space (estimate based on duration if available)
    AVStream* stream = formatCtx->streams[audioStreamIndex];
    if (stream->duration != AV_NOPTS_VALUE) {
        int64_t estimatedSamples = av_rescale_q(stream->duration,
            stream->time_base, {1, codecCtx->sample_rate});
        outData.samples.reserve(estimatedSamples * outData.channels);
    }

    // Main decoding loop
    while (av_read_frame(formatCtx, packet) >= 0) {
        if (packet->stream_index == audioStreamIndex) {
            // Send packet to decoder
            int ret = avcodec_send_packet(codecCtx, packet);
            if (ret < 0 && ret != AVERROR(EAGAIN)) {
                av_packet_unref(packet);
                char errbuf[128];
                av_strerror(ret, errbuf, sizeof(errbuf));
                errorMsg = "Error sending packet: " + std::string(errbuf);
                return false;
            }

            // Receive all available frames
            while (ret >= 0) {
                ret = avcodec_receive_frame(codecCtx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                if (ret < 0) {
                    av_packet_unref(packet);
                    char errbuf[128];
                    av_strerror(ret, errbuf, sizeof(errbuf));
                    errorMsg = "Error receiving frame: " + std::string(errbuf);
                    return false;
                }

                // Process decoded frame
                processFrame(frame, outData.samples);
                outData.totalSamples += frame->nb_samples;

                av_frame_unref(frame);
            }
        }
        av_packet_unref(packet);
    }

    // Flush decoder (drain remaining frames)
    avcodec_send_packet(codecCtx, nullptr);
    int ret;
    while ((ret = avcodec_receive_frame(codecCtx, frame)) == 0) {
        processFrame(frame, outData.samples);
        outData.totalSamples += frame->nb_samples;
        av_frame_unref(frame);
    }

    return true;
}

float AudioLoader::getSample(const AVCodecContext* ctx, uint8_t* buffer, int sampleIndex) {
    int sampleSize = av_get_bytes_per_sample(ctx->sample_fmt);
    float result = 0.0f;

    switch (ctx->sample_fmt) {
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_U8P: {
            uint8_t val = buffer[sampleIndex];
            result = (val - 128) / 128.0f;
            break;
        }
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S16P: {
            int16_t val = ((int16_t*)buffer)[sampleIndex];
            result = val / 32768.0f;
            break;
        }
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P: {
            int32_t val = ((int32_t*)buffer)[sampleIndex];
            result = val / 2147483648.0f;
            break;
        }
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP: {
            result = ((float*)buffer)[sampleIndex];
            break;
        }
        case AV_SAMPLE_FMT_DBL:
        case AV_SAMPLE_FMT_DBLP: {
            result = (float)((double*)buffer)[sampleIndex];
            break;
        }
        default:
            result = 0.0f;
    }

    return result;
}

void AudioLoader::processFrame(const AVFrame* frame, std::vector<float>& outSamples) {
    int nbSamples = frame->nb_samples;
    int channels = codecCtx->ch_layout.nb_channels;

    // Check if format is planar (each channel in separate buffer)
    bool isPlanar = av_sample_fmt_is_planar(codecCtx->sample_fmt);

    if (isPlanar) {
        // Planar: data[0] = left channel, data[1] = right channel, etc.
        for (int s = 0; s < nbSamples; s++) {
            for (int c = 0; c < channels; c++) {
                float sample = getSample(codecCtx, frame->extended_data[c], s);
                outSamples.push_back(sample);
            }
        }
    } else {
        // Packed: all channels interleaved in data[0]
        for (int s = 0; s < nbSamples * channels; s++) {
            float sample = getSample(codecCtx, frame->extended_data[0], s);
            outSamples.push_back(sample);
        }
    }
}

bool AudioLoader::getFileInfo(const std::string& filepath, int& sampleRate,
                              int& channels, int64_t& totalSamples, std::string& errorMsg) {
    cleanup();

    if (!openFile(filepath, errorMsg)) return false;
    if (!findAudioStream(errorMsg)) return false;
    if (!openCodec(errorMsg)) return false;

    sampleRate = codecCtx->sample_rate;
    channels = codecCtx->ch_layout.nb_channels;

    // Calculate total samples from duration
    AVStream* stream = formatCtx->streams[audioStreamIndex];
    if (stream->duration != AV_NOPTS_VALUE) {
        totalSamples = av_rescale_q(stream->duration,
            stream->time_base, {1, codecCtx->sample_rate});
    } else {
        totalSamples = 0;  // Unknown
    }

    cleanup();
    return true;
}

} // namespace vgLib_v2
