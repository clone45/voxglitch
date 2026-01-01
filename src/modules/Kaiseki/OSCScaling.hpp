#pragma once

namespace OSCScaling {
    // Pitch scaling constants (in octaves/V-Oct)
    constexpr float PITCH_MIN_OCTAVES = -2.0f;  // Minimum pitch in octaves
    constexpr float PITCH_MAX_OCTAVES = 2.0f;   // Maximum pitch in octaves
    constexpr float PITCH_RANGE = PITCH_MAX_OCTAVES - PITCH_MIN_OCTAVES;
    
    // Volume scaling constants (in dB)
    constexpr float VOLUME_MIN_DB = -60.0f;
    constexpr float VOLUME_MAX_DB = 0.0f;
    
    // Pan scaling constants
    constexpr float PAN_MIN = -1.0f;
    constexpr float PAN_MAX = 1.0f;
    
    // BPM scaling constants
    constexpr float BPM_MIN = 20.0f;
    constexpr float BPM_MAX = 300.0f;
    
    // Generic scaling function that maps a value from one range to another
    inline float scale(float value, float inMin, float inMax, float outMin, float outMax) {
        // Clamp input value to input range
        if (value <= inMin) return outMin;
        if (value >= inMax) return outMax;
        
        // Linear scaling formula
        float normalized = (value - inMin) / (inMax - inMin);
        return outMin + (normalized * (outMax - outMin));
    }
    
    // Convenience function for scaling OSC values (0-1) to pitch range
    inline float scalePitch(float oscValue) {
        return scale(oscValue, 0.0f, 1.0f, PITCH_MIN_OCTAVES, PITCH_MAX_OCTAVES);
    }
    
    // Convenience function for scaling OSC values (0-1) to volume (linear)
    inline float scaleVolume(float oscValue) {
        // Direct linear scaling: 0.0 = 0% volume, 0.5 = 50% volume, 1.0 = 100% volume
        return oscValue;
    }
    
    // Convenience function for scaling OSC values (0-1) to pan (-1 to 1)
    inline float scalePan(float oscValue) {
        return scale(oscValue, 0.0f, 1.0f, PAN_MIN, PAN_MAX);
    }
    
    // Convenience function for scaling OSC values (0-1) to BPM
    inline float scaleBPM(float oscValue) {
        return scale(oscValue, 0.0f, 1.0f, BPM_MIN, BPM_MAX);
    }
}