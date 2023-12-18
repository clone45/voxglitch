#include "rack.hpp" // Assuming this is needed for the VCV Rack environment

class SlewLimiter {
private:
    float currentOutput = 0.0f;
    float riseTime = 0.0f;  // Time (in seconds) to reach the full value (default immediate)
    float fallTime = 0.0f;  // Time (in seconds) to reach zero (default immediate)

    float riseRate = INFINITY;  // Default to infinite rate (immediate)
    float fallRate = INFINITY;  // Default to infinite rate (immediate)

public:
    float process(float input) {
        float deltaTime = APP->engine->getSampleTime();

        if (riseTime > 0.0f) {
            riseRate = deltaTime / riseTime;
        } else {
            riseRate = INFINITY;
        }

        if (fallTime > 0.0f) {
            fallRate = deltaTime / fallTime;
        } else {
            fallRate = INFINITY;
        }

        float delta = input - currentOutput;

        if (delta > 0) {
            delta = std::min(delta, riseRate * delta);
        } else {
            delta = std::max(delta, fallRate * delta);
        }

        currentOutput += delta;
        return currentOutput;
    }

    void setRiseFallTimes(float rise, float fall) {
        riseTime = rise;
        fallTime = fall;
    }
};
