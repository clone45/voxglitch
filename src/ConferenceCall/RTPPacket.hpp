#pragma once

#include <cstdint>
#include <vector>

class RTPPacket {
public:
    RTPPacket(uint8_t payloadType, uint16_t sequenceNumber, uint32_t timestamp, uint32_t ssrc, const std::vector<uint8_t>& payload)
        : payloadType(payloadType), sequenceNumber(sequenceNumber), timestamp(timestamp), ssrc(ssrc), payload(payload) {}

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> packet;
        packet.reserve(12 + payload.size());  // 12 bytes header + payload

        // RTP version (2), padding (0), extension (0), CSRC count (0)
        packet.push_back(0x80);  

        // Marker bit (0) and payload type
        packet.push_back(payloadType & 0x7F);  

        // Sequence number (16 bits)
        packet.push_back((sequenceNumber >> 8) & 0xFF);
        packet.push_back(sequenceNumber & 0xFF);

        // Timestamp (32 bits)
        for (int i = 24; i >= 0; i -= 8) {
            packet.push_back((timestamp >> i) & 0xFF);
        }

        // SSRC (32 bits)
        for (int i = 24; i >= 0; i -= 8) {
            packet.push_back((ssrc >> i) & 0xFF);
        }

        // Payload
        packet.insert(packet.end(), payload.begin(), payload.end());

        return packet;
    }

    static RTPPacket deserialize(const std::vector<uint8_t>& data) {
        if (data.size() < 12) {
            throw std::runtime_error("Invalid RTP packet: too short");
        }

        uint8_t payloadType = data[1] & 0x7F;
        uint16_t sequenceNumber = (data[2] << 8) | data[3];
        uint32_t timestamp = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
        uint32_t ssrc = (data[8] << 24) | (data[9] << 16) | (data[10] << 8) | data[11];

        std::vector<uint8_t> payload(data.begin() + 12, data.end());

        return RTPPacket(payloadType, sequenceNumber, timestamp, ssrc, payload);
    }

    uint8_t getPayloadType() const { return payloadType; }
    uint16_t getSequenceNumber() const { return sequenceNumber; }
    uint32_t getTimestamp() const { return timestamp; }
    uint32_t getSSRC() const { return ssrc; }
    const std::vector<uint8_t>& getPayload() const { return payload; }

private:
    uint8_t payloadType;
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
    std::vector<uint8_t> payload;
};