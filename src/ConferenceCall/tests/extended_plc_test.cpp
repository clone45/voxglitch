
// Steps to run this test
// 1. Compile using g++: `g++ -o extended_plc_test extended_plc_test.cpp ../PLCProcessor.cpp`
// 2. Run using `./extended_plc_test`

// In case you aren't aware what PLCProcessing is, it's a class that takes a stream of RTP packets, 
// and if packets are lost, it tries to estimate the missing data using various algorithms.  In short, 
// it's a class that does some form of interpolation on the data to patch the holes.

// The PLCProcessor is tested with different strategies: OFF, BASIC, and ADAPTIVE.
// The test involves processing packets with known patterns of loss and observing the output.


#include <iostream>
#include <vector>
#include <cstdint>
#include <utility>
#include "../PLCProcessor.hpp"

void printVector(const std::vector<uint8_t>& vec) {
    for (uint8_t val : vec) {
        std::cout << static_cast<int>(val) << " ";
    }
    std::cout << std::endl;
}

void runTest(PLCProcessor& plc, const std::vector<std::pair<std::vector<uint8_t>, bool>>& testData, const std::string& testName) {
    std::cout << "\nRunning test: " << testName << std::endl;
    for (const auto& data : testData) {
        const std::vector<uint8_t>& packet = data.first;
        bool isLost = data.second;
        std::cout << (isLost ? "Lost: " : "Good: ");
        printVector(plc.processPacket(packet, isLost));
    }
}

int main() {
    PLCProcessor plc;

    // Test 1: Consecutive packet loss
    std::vector<std::pair<std::vector<uint8_t>, bool>> consecutiveLossTest = {
        std::make_pair(std::vector<uint8_t>{1, 2, 3, 4}, false),
        std::make_pair(std::vector<uint8_t>{5, 6, 7, 8}, true),
        std::make_pair(std::vector<uint8_t>{9, 10, 11, 12}, true),
        std::make_pair(std::vector<uint8_t>{13, 14, 15, 16}, false)
    };

    // Test 2: Varying packet sizes
    std::vector<std::pair<std::vector<uint8_t>, bool>> varyingSizesTest = {
        std::make_pair(std::vector<uint8_t>{1, 2}, false),
        std::make_pair(std::vector<uint8_t>{3, 4, 5, 6, 7}, false),
        std::make_pair(std::vector<uint8_t>{8}, true),
        std::make_pair(std::vector<uint8_t>{9, 10, 11}, false)
    };

    // Test 3: Longer sequence
    std::vector<std::pair<std::vector<uint8_t>, bool>> longerSequenceTest;
    for (int i = 0; i < 20; ++i) {
        longerSequenceTest.push_back(std::make_pair(std::vector<uint8_t>{static_cast<uint8_t>(i)}, i % 5 == 0));
    }

    // Test 4: Edge cases
    std::vector<std::pair<std::vector<uint8_t>, bool>> edgeCasesTest = {
        std::make_pair(std::vector<uint8_t>{}, false),  // Empty packet
        std::make_pair(std::vector<uint8_t>{1}, true),
        std::make_pair(std::vector<uint8_t>{2}, false),
        std::make_pair(std::vector<uint8_t>{3}, true),
        std::make_pair(std::vector<uint8_t>{4}, false),
        std::make_pair(std::vector<uint8_t>{5}, true)
    };

    // Run tests with OFF strategy
    plc.setStrategy(PLCStrategy::OFF);
    runTest(plc, consecutiveLossTest, "Consecutive Loss (OFF)");
    runTest(plc, varyingSizesTest, "Varying Sizes (OFF)");
    runTest(plc, longerSequenceTest, "Longer Sequence (OFF)");
    runTest(plc, edgeCasesTest, "Edge Cases (OFF)");

    // Run tests with BASIC strategy
    plc.setStrategy(PLCStrategy::BASIC);
    runTest(plc, consecutiveLossTest, "Consecutive Loss (BASIC)");
    runTest(plc, varyingSizesTest, "Varying Sizes (BASIC)");
    runTest(plc, longerSequenceTest, "Longer Sequence (BASIC)");
    runTest(plc, edgeCasesTest, "Edge Cases (BASIC)");

    // Test 5: Strategy switching mid-stream
    std::cout << "\nRunning test: Strategy Switching" << std::endl;
    plc.setStrategy(PLCStrategy::OFF);
    std::cout << "OFF: " << std::endl;
    printVector(plc.processPacket(std::vector<uint8_t>{1, 2, 3, 4}, false));
    printVector(plc.processPacket(std::vector<uint8_t>{5, 6, 7, 8}, true));
    plc.setStrategy(PLCStrategy::BASIC);
    std::cout << "Switched to BASIC: " << std::endl;
    printVector(plc.processPacket(std::vector<uint8_t>{9, 10, 11, 12}, true));
    printVector(plc.processPacket(std::vector<uint8_t>{13, 14, 15, 16}, false));

    return 0;
}

/* Expected output

$ ./extended_plc_test.exe

Running test: Consecutive Loss (OFF)
Good: 1 2 3 4
Lost: 0 0 0 0
Lost: 0 0 0 0
Good: 13 14 15 16

Running test: Varying Sizes (OFF)
Good: 1 2
Good: 3 4 5 6 7
Lost: 0 0 0 0 0
Good: 9 10 11

Running test: Longer Sequence (OFF)
Lost: 0 0 0
Good: 1
Good: 2
Good: 3
Good: 4
Lost: 0
Good: 6
Good: 7
Good: 8
Good: 9
Lost: 0
Good: 11
Good: 12
Good: 13
Good: 14
Lost: 0
Good: 16
Good: 17
Good: 18
Good: 19

Running test: Edge Cases (OFF)
Good:
Lost:
Good: 2
Lost: 0
Good: 4
Lost: 0

Running test: Consecutive Loss (BASIC)
Good: 1 2 3 4
Lost: 1 2 3 4
Lost: 1 2 3 4
Good: 13 14 15 16

Running test: Varying Sizes (BASIC)
Good: 1 2
Good: 3 4 5 6 7
Lost: 3 4 5 6 7
Good: 9 10 11

Running test: Longer Sequence (BASIC)
Lost: 9 10 11
Good: 1
Good: 2
Good: 3
Good: 4
Lost: 4
Good: 6
Good: 7
Good: 8
Good: 9
Lost: 9
Good: 11
Good: 12
Good: 13
Good: 14
Lost: 14
Good: 16
Good: 17
Good: 18
Good: 19

Running test: Edge Cases (BASIC)
Good:
Lost:
Good: 2
Lost: 2
Good: 4
Lost: 4

Running test: Strategy Switching
OFF:
1 2 3 4
0 0 0 0
Switched to BASIC:
1 2 3 4
13 14 15 16


*/