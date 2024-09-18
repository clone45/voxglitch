#pragma once

#include "RTPPacket.hpp"
#include "RTPSession.hpp"
#include <cstdint>
#include <vector>
#include <deque>
#include <random>

class UDPNetworkSimulator {
public:
    UDPNetworkSimulator(int sampleRate = 48000, int frameDuration = 20)
        : sampleRate(sampleRate), frameDuration(frameDuration),
          samplesPerFrame(sampleRate * frameDuration / 1000),
          rng(std::random_device{}()), uniformDist(0.0f, 1.0f) {}

    void setDropoutRate(float rate) { dropoutRate = rate; }
    void setLatency(float ms) { latency = ms; }
    void setJitter(float amount) { jitter = amount; }

    void post(const RTPPacket& packet) {
        packetBuffer.push_back(packet);
    }

    void process() {
        std::vector<RTPPacket> processedPackets;
        for (const auto& packet : packetBuffer) {
            if (uniformDist(rng) >= dropoutRate) {
                processedPackets.push_back(packet);
            }
        }
       
        // Simulate jitter by reordering packets
        if (jitter > 0) {
            std::shuffle(processedPackets.begin(), processedPackets.end(), rng);
        }

        // Simulate latency
        if (latency > 0) {
            // In a real implementation, you'd delay packets here
        }

        receivedPackets.insert(receivedPackets.end(), processedPackets.begin(), processedPackets.end());
        packetBuffer.clear();
    }

    RTPPacket get() {
        if (receivedPackets.empty()) {
            // Return an empty packet or throw an exception
            return RTPPacket(0, 0, 0, 0, std::vector<uint8_t>());
        }
        auto packet = receivedPackets.front();
        receivedPackets.pop_front();
        return packet;
    }

private:
    int sampleRate;
    int frameDuration;  // in milliseconds
    int samplesPerFrame;
    std::deque<RTPPacket> packetBuffer;
    std::deque<RTPPacket> receivedPackets;
    float dropoutRate = 0.0f;
    float latency = 0.0f;
    float jitter = 0.0f;
    std::mt19937 rng;
    std::uniform_real_distribution<float> uniformDist;
};