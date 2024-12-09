#pragma once

enum class MarkerType {
    OUTPUT,  // Original trigger output markers
    JUMP     // New jump position markers
};

struct Marker {
    int output_number;  // Which output (0-31) this marker triggers
    MarkerType type;
    
    // Constructor
    Marker(int out, MarkerType t = MarkerType::OUTPUT) 
        : output_number(out), type(t) {}
};