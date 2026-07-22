// pragm once:
#pragma once

struct Marker {
    int output_number;  // Which output (0-31) this marker triggers
    
    // Constructor
    Marker(int out) : output_number(out) {}
};