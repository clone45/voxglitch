class AdaptiveGainControl {
private:
    float gain = 1.0f;
    float targetLevel = 0.7f;
    float adjustmentSpeed = 0.001f;
    float averageLevel = 0.0f;
    float smoothingFactor = 0.001f;
    const float minGain = 0.1f;
    const float maxGain = 10.0f;

public:
    void process(float& inputLeft, float& inputRight) {
        // Compute the current level
        float currentLevel = (std::fabs(inputLeft) + std::fabs(inputRight)) / 2.0f;

        // Update exponential moving average
        averageLevel = (smoothingFactor * currentLevel) + ((1.0f - smoothingFactor) * averageLevel);

        // Prevent very low average levels
        if (averageLevel < 1e-6f) {
            averageLevel = 1e-6f;
        }

        // Calculate desired gain
        float desiredGain = targetLevel / averageLevel;

        // Limit gain change per call
        float maxGainChangePerCall = 0.01f;
        float gainChange = adjustmentSpeed * (desiredGain - gain);
        gainChange = rack::math::clamp(gainChange, -maxGainChangePerCall, maxGainChangePerCall);
        gain += gainChange;

        // Clamp gain to min and max values
        gain = rack::math::clamp(gain, minGain, maxGain);

        // Apply the gain
        inputLeft *= gain;
        inputRight *= gain;
    }

    // Optional: Set parameters if you want to expose them
    void setParameters(float smoothingFactorParam, float adjustmentSpeedParam) {
        smoothingFactor = smoothingFactorParam;
        adjustmentSpeed = adjustmentSpeedParam;
    }
};