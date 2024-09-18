#include <cstdint>
#include <vector>
#include <deque>
#include <random>

struct RTPHeader {
    uint16_t sequenceNumber;
    uint32_t timestamp;
    // Other fields omitted for simplicity
};

struct UDPPacket {
    RTPHeader header;
    std::vector<uint8_t> payload;
};

class UDPNetworkSimulator {
public:
    UDPNetworkSimulator(int sampleRate = 48000, int frameDuration = 20)
        : sampleRate(sampleRate), frameDuration(frameDuration), 
          samplesPerFrame(sampleRate * frameDuration / 1000),
          rng(std::random_device{}()), uniformDist(0.0f, 1.0f) {
        sequenceNumber = 0;
        timestamp = 0;
    }

    void setDropoutRate(float rate) { dropoutRate = rate; }
    void setLatency(float ms) { latency = ms; }
    void setJitter(float amount) { jitter = amount; }

    void post(const std::vector<uint8_t>& leftData, const std::vector<uint8_t>& rightData) {
        UDPPacket packet;
        packet.header.sequenceNumber = sequenceNumber++;
        packet.header.timestamp = timestamp;
        timestamp += samplesPerFrame;

        // Interleave left and right channel data
        packet.payload.reserve(leftData.size() + rightData.size());
        for (size_t i = 0; i < leftData.size(); i++) {
            packet.payload.push_back(leftData[i]);
            if (i < rightData.size()) {
                packet.payload.push_back(rightData[i]);
            }
        }

        packetBuffer.push_back(packet);
    }

    void process() {
        std::vector<UDPPacket> processedPackets;
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

    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> get() {
        if (receivedPackets.empty()) {
            return {{}, {}};
        }

        auto packet = receivedPackets.front();
        receivedPackets.pop_front();

        std::vector<uint8_t> leftData, rightData;
        for (size_t i = 0; i < packet.payload.size(); i += 2) {
            leftData.push_back(packet.payload[i]);
            if (i + 1 < packet.payload.size()) {
                rightData.push_back(packet.payload[i + 1]);
            }
        }

        return {leftData, rightData};
    }

private:
    int sampleRate;
    int frameDuration;  // in milliseconds
    int samplesPerFrame;
    uint16_t sequenceNumber;
    uint32_t timestamp;
    std::deque<UDPPacket> packetBuffer;
    std::deque<UDPPacket> receivedPackets;
    float dropoutRate = 0.0f;
    float latency = 0.0f;
    float jitter = 0.0f;
    std::mt19937 rng;
    std::uniform_real_distribution<float> uniformDist;
};