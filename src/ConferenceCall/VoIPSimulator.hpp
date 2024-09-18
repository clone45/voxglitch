#pragma once

#include "ADPCMEffectsWrapper.hpp"
#include "UDPNetworkSimulator.hpp"
#include "RTPSession.hpp"
#include <vector>

class VoIPSimulator {
public:
    VoIPSimulator(int sampleRate = 48000, int frameDuration = 20)
        : adpcmProcessor(),
          adpcmWrapper(adpcmProcessor),
          udp(sampleRate, frameDuration),
          rtpSession(static_cast<uint32_t>(std::random_device{}()))
    {
        inputBufferLeft.reserve(BlockADPCMProcessor::BLOCK_SIZE);
        inputBufferRight.reserve(BlockADPCMProcessor::BLOCK_SIZE);
    }

    void setNetworkParams(float dropoutRate, float latency, float jitter) {
        udp.setDropoutRate(dropoutRate);
        udp.setLatency(latency);
        udp.setJitter(jitter);
    }

    void pushSamples(float left, float right) {
        inputBufferLeft.push_back(left);
        inputBufferRight.push_back(right);
    }

    bool hasOutput() const {
        return !outputBufferLeft.empty() && !outputBufferRight.empty();
    }

    std::pair<float, float> popOutputSamples() {
        if (!hasOutput()) {
            return {0.f, 0.f};
        }
        float left = outputBufferLeft.front();
        float right = outputBufferRight.front();
        outputBufferLeft.erase(outputBufferLeft.begin());
        outputBufferRight.erase(outputBufferRight.begin());
        return {left, right};
    }

    void process() {
        if (inputBufferLeft.size() == BlockADPCMProcessor::BLOCK_SIZE) {
            std::vector<uint8_t> compressedLeft = adpcmWrapper.compress(inputBufferLeft);
            std::vector<uint8_t> compressedRight = adpcmWrapper.compress(inputBufferRight);

            std::vector<uint8_t> payload = interleave(compressedLeft, compressedRight);

            RTPPacket packet = rtpSession.createPacket(payload);

            udp.post(packet);
            udp.process();

            RTPPacket receivedPacket = udp.get();
            rtpSession.processReceivedPacket(receivedPacket);

            if (!receivedPacket.getPayload().empty()) {
                std::pair<std::vector<uint8_t>, std::vector<uint8_t>> deinterleaved = deinterleave(receivedPacket.getPayload());
                std::vector<uint8_t>& receivedLeft = deinterleaved.first;
                std::vector<uint8_t>& receivedRight = deinterleaved.second;

                std::vector<float> decompressedLeft = adpcmWrapper.decompress(receivedLeft);
                std::vector<float> decompressedRight = adpcmWrapper.decompress(receivedRight);

                outputBufferLeft.insert(outputBufferLeft.end(), decompressedLeft.begin(), decompressedLeft.end());
                outputBufferRight.insert(outputBufferRight.end(), decompressedRight.begin(), decompressedRight.end());
            }

            inputBufferLeft.clear();
            inputBufferRight.clear();
        }
    }

private:
    BlockADPCMProcessor adpcmProcessor;
    ADPCMEffectsWrapper adpcmWrapper;
    UDPNetworkSimulator udp;
    RTPSession rtpSession;

    std::vector<float> inputBufferLeft;
    std::vector<float> inputBufferRight;
    std::vector<float> outputBufferLeft;
    std::vector<float> outputBufferRight;

    std::vector<uint8_t> interleave(const std::vector<uint8_t>& left, const std::vector<uint8_t>& right) {
        std::vector<uint8_t> result;
        result.reserve(left.size() + right.size());
        for (size_t i = 0; i < left.size(); ++i) {
            result.push_back(left[i]);
            if (i < right.size()) {
                result.push_back(right[i]);
            }
        }
        return result;
    }

    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> deinterleave(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> left, right;
        for (size_t i = 0; i < data.size(); i += 2) {
            left.push_back(data[i]);
            if (i + 1 < data.size()) {
                right.push_back(data[i + 1]);
            }
        }
        return {left, right};
    }
};