#pragma once
#include <vector>

struct ScrubState {
    static const size_t SCRUB_BUFFER_SIZE = 256 * 16;
    
    std::vector<float> buffer_left;
    std::vector<float> buffer_right;
    unsigned int buffer_position = 0;    // For audio output
    unsigned int display_position = 0;   // For UI/playhead display
    unsigned int target_position = 0;    // Position from drag events
    float scrub_speed = 0.0f;
    bool buffer_needs_update = false;
    
    ScrubState() : buffer_left(SCRUB_BUFFER_SIZE), buffer_right(SCRUB_BUFFER_SIZE) {}
    
    void reset() {
        buffer_position = 0;
        display_position = 0;
        target_position = 0;
        scrub_speed = 0.0f;
        buffer_needs_update = true;
    }
};