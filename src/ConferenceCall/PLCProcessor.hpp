// PLCProcessor.hpp
#pragma once

#include <vector>
#include <cstdint>

enum class PLCStrategy {
    OFF,
    BASIC,
    // Future strategies will be added here
    // WAVEFORM_SUBSTITUTION,
    // PITCH_WAVEFORM_REPLICATION,
    // etc.
};

class PLCProcessor {
public:
    PLCProcessor() : lastGoodPacket(), consecutiveLostPackets(0), currentStrategy(PLCStrategy::OFF) {}

    void setStrategy(PLCStrategy strategy) {
        currentStrategy = strategy;
    }

    std::vector<uint8_t> processPacket(const std::vector<uint8_t>& packet, bool packetLost) {
        if (packetLost) {
            consecutiveLostPackets++;
            return conceal();
        } else {
            lastGoodPacket = packet;
            consecutiveLostPackets = 0;
            return packet;
        }
    }

private:
    std::vector<uint8_t> lastGoodPacket;
    int consecutiveLostPackets;
    PLCStrategy currentStrategy;

    std::vector<uint8_t> conceal() {
        switch (currentStrategy) {
            case PLCStrategy::OFF:
                return std::vector<uint8_t>(lastGoodPacket.size(), 0);  // Return silence
            case PLCStrategy::BASIC:
                return lastGoodPacket;  // Simple repetition
            // Future strategies will be added here
            default:
                return lastGoodPacket;
        }
    }
};