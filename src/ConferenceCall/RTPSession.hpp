#pragma once

class RTPSession {
public:
    RTPSession(uint32_t ssrc) : ssrc(ssrc), sequenceNumber(0), timestamp(0) {}

    RTPPacket createPacket(const std::vector<uint8_t>& payload) {
        RTPPacket packet(96, sequenceNumber++, timestamp, ssrc, payload);
        timestamp += 160;  // Assuming 20ms frames at 8kHz
        return packet;
    }

    void processReceivedPacket(const RTPPacket& packet) {
        // Here you could implement logic for handling out-of-order packets,
        // detecting packet loss, etc.
    }

private:
    uint32_t ssrc;
    uint16_t sequenceNumber;
    uint32_t timestamp;
};