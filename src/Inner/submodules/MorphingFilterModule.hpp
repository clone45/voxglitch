// MorphingMultiModeFilterModule.hpp

#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"

class MorphingFilterModule : public BaseModule
{
private:

    // Filter state variables
    float lp_state = 0.0f;
    float bp_state = 0.0f;
    float hp_state = 0.0f;
    float lp_output = 0.0f;

public:

    enum INPUTS {
        AUDIO,
        CUTOFF,
        RESONANCE,
        MORPH,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        CUTOFF,
        MORPH,
        RESONANCE,
        NUM_PARAMS
    };

    MorphingFilterModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS); 

        config[CUTOFF].setValue(0.0f);
        config[RESONANCE].setValue(0.0f);
        config[MORPH].setValue(0.0f);
    }

    void process(unsigned int sample_rate) override
    {
        float audio_input_voltage = inputs[AUDIO]->isConnected() ? inputs[AUDIO]->getVoltage() : 0.0;
        float cutoff_input_voltage = inputs[CUTOFF]->isConnected() ? inputs[CUTOFF]->getVoltage() : params[CUTOFF].getValue();
        float resonance_input_voltage = inputs[RESONANCE]->->isConnected() ? inputs[RESONANCE]->getVoltage() : params[RESONANCE].getValue();
        float morph_input_voltage = inputs[MORPH]->isConnected() ? inputs[MORPH]->getVoltage() : params[MORPH].getValue();

        float cutoff_input_value = std::min(cutoff_input_voltage * 2200.0f, 0.49f * sample_rate);

        float fc = cutoff_input_value / (float)sample_rate; // Normalize cutoff frequency
        float q = 1.0f / (1.0f + resonance_input_voltage * fc);

        // Calculate filter coefficients
        float g = std::tan(3.14159265358979323846 * fc);
        float a1 = 1.0f / (1.0f + g * q + g * g);
        float a2 = 2.0f * (g * g - 1.0f) * a1;
        float a3 = (1.0f - g * q + g * g) * a1;

        // State-variable filter equations
        lp_state = a1 * audio_input_voltage + a2 * lp_state + a3 * hp_state;
        lp_state = clamp(lp_state, -1.0f, 1.0f); // Clamp lp_state to avoid instability

        bp_state = g * q * (audio_input_voltage - lp_state) + bp_state;
        bp_state = clamp(bp_state, -1.0f, 1.0f); // Clamp bp_state to avoid instability

        hp_state = audio_input_voltage - lp_state - q * bp_state;
        hp_state = clamp(hp_state, -1.0f, 1.0f); // Clamp hp_state to avoid instability

        // Calculate the crossfade factors
        float mix = morph_input_voltage / 10.0f;
        float lp_factor = std::max(0.0f, 1.0f - 4.0f * std::abs(mix - 0.25f));
        float hp_factor = std::max(0.0f, 1.0f - 4.0f * std::abs(mix - 0.75f));
        float bp_factor = 1.0f - lp_factor - hp_factor;

        // Crossfade between the three filter outputs
        float output = lp_state * lp_factor + bp_state * bp_factor + hp_state * hp_factor;
        outputs[OUTPUT]->setVoltage(output);
    }   
};
