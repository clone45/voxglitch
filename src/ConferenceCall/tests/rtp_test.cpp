// Steps to run this test
// 1. Compile using g++: g++ -std=c++11 -o rtp_test rtp_test.cpp -I..
// 2. Run using `./rtp_test`

#include <iostream>
#include <vector>
#include <cassert>
#include "../RTPPacket.hpp"
#include "../RTPSession.hpp"

void printPacketInfo(const RTPPacket& packet) {
    std::cout << "Sequence Number: " << packet.getSequenceNumber()
              << ", Timestamp: " << packet.getTimestamp()
              << ", SSRC: " << packet.getSSRC()
              << ", Payload size: " << packet.getPayload().size() << std::endl;
}

void testRTPPacket() {
    std::cout << "Testing RTPPacket..." << std::endl;

    // Test packet creation and getters
    std::vector<uint8_t> testPayload = {1, 2, 3, 4};
    RTPPacket packet(96, 1000, 50000, 12345, testPayload);
    assert(packet.getPayloadType() == 96);
    assert(packet.getSequenceNumber() == 1000);
    assert(packet.getTimestamp() == 50000);
    assert(packet.getSSRC() == 12345);
    assert(packet.getPayload() == testPayload);

    // Test serialization and deserialization
    std::vector<uint8_t> serialized = packet.serialize();
    RTPPacket deserialized = RTPPacket::deserialize(serialized);
    assert(packet.getPayloadType() == deserialized.getPayloadType());
    assert(packet.getSequenceNumber() == deserialized.getSequenceNumber());
    assert(packet.getTimestamp() == deserialized.getTimestamp());
    assert(packet.getSSRC() == deserialized.getSSRC());
    assert(packet.getPayload() == deserialized.getPayload());

    std::cout << "RTPPacket tests passed!" << std::endl;
}

void testRTPSession() {
    std::cout << "Testing RTPSession..." << std::endl;

    RTPSession session(54321);  // SSRC: 54321

    // Test packet creation
    std::vector<uint8_t> payload1 = {1, 2, 3, 4};
    RTPPacket packet1 = session.createPacket(payload1);
    printPacketInfo(packet1);

    std::vector<uint8_t> payload2 = {5, 6, 7, 8};
    RTPPacket packet2 = session.createPacket(payload2);
    printPacketInfo(packet2);

    // Check sequence number increment
    assert(packet2.getSequenceNumber() == packet1.getSequenceNumber() + 1);

    // Check timestamp increment (assuming default 8000 Hz sample rate and 160 samples per packet)
    assert(packet2.getTimestamp() == packet1.getTimestamp() + 160);

    // Test packet processing
    session.processReceivedPacket(packet1);
    session.processReceivedPacket(packet2);

    // Test out-of-order packet
    RTPPacket outOfOrderPacket(96, packet1.getSequenceNumber() - 1, packet1.getTimestamp() - 160, 54321, {9, 10, 11, 12});
    session.processReceivedPacket(outOfOrderPacket);

    // Test duplicate packet
    session.processReceivedPacket(packet1);

    std::cout << "RTPSession tests passed!" << std::endl;
}

int main() {
    testRTPPacket();
    std::cout << std::endl;
    testRTPSession();
    return 0;
}