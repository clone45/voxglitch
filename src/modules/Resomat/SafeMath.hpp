#pragma once
#include <cstdint>

// Safe math operations for bytebeat equations
// Prevents division by zero and other unsafe operations
namespace SafeMath {
    // Safe division that avoids division by zero
    inline uint32_t div(uint32_t a, uint32_t b) {
        if (b == 0) return 0;
        return a / b;
    }

    // Safe modulo that avoids division by zero
    inline uint32_t mod(uint32_t a, uint32_t b) {
        if (b == 0) return 0;
        return a % b;
    }
}
