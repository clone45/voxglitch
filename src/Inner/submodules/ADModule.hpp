#pragma once

#include "../BaseModule.hpp"

class ADModule : public BaseModule
{
public:
    enum INPUTS {
        GATE,
        ATTACK,
        DECAY,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        NUM_PARAMS
    };

    ADModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        float gate = inputs[GATE]->getVoltage();
        float attack_voltage = inputs[ATTACK]->getVoltage();
        float decay_voltage = inputs[DECAY]->getVoltage();

        attack_voltage = clamp(attack_voltage, 0.0f, 10.0f);
        decay_voltage = clamp(decay_voltage, 0.0f, 10.0f);

        float output = 0.0f;

        // If gate is high, set output to attack level. Otherwise, set output to decay level.
        if (gate > 0.0f) {
            float attack_time = map(attack_voltage, 0.0f, 10.0f, 0.001f, 10.0f);
            float attack_phase = std::min(phase / attack_time, 1.0f);
            output = attack_phase;
        }
        else {
            float decay_time = map(decay_voltage, 0.0f, 10.0f, 0.001f, 10.0f);
            float decay_phase = std::min(phase / decay_time, 1.0f);
            output = 1.0f - decay_phase;
        }

        // Increment the phase
        phase += 1.0f / static_cast<float>(sample_rate * 5.0f);

        // Ensure that the phase stays between 0 and 1
        if (phase >= 1.0f) phase = 1.0f;

        outputs[OUTPUT]->setVoltage(output * 10.0f);
    }

private:
    float phase = 0.0f;

    float map(float value, float in_min, float in_max, float out_min, float out_max) {
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
};
