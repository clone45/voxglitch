#pragma once
#include <vector>

struct ScrubState {
    static const size_t SCRUB_BUFFER_SIZE = 16384;  // Adjustable size
    
    std::vector<float> buffer_left;
    std::vector<float> buffer_right;
    unsigned int buffer_position = 0;    // Current position in buffer
    unsigned int target_position = 0;    // Target sample position from scrubber
    float scrub_speed = 0.0f;           // Speed/direction of scrubbing
    bool buffer_needs_update = false;    // Flag to refill buffer
    
    ScrubState() : buffer_left(SCRUB_BUFFER_SIZE), buffer_right(SCRUB_BUFFER_SIZE) {}
    
    void reset() {
        buffer_position = 0;
        target_position = 0;
        scrub_speed = 0.0f;
        buffer_needs_update = true;
    }
};