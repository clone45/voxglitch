// MorphingMultiModeFilterModule.hpp

#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"

class MorphingFilterModule : public IModule
{
private:

    // Filter state variables
    float lp_state = 0.0f;
    float bp_state = 0.0f;
    float hp_state = 0.0f;
    float lp_output = 0.0f;

public:

    Sport *audio_input_port = new Sport(this);
    Sport *cutoff_input_port = new Sport(this);
    Sport *resonance_input_port = new Sport(this);
    Sport *morph_input_port = new Sport(this);
    Sport *audio_output_port = new Sport(this);

    MorphingFilterModule()
    {
        setParameter("cutoff", 0.0f);
        setParameter("morph", 0.0f);
        setParameter("resonance", 0.0f);        
    }

    void process(unsigned int sample_rate) override
    {
        float audio_input_voltage = audio_input_port->isConnected() ? audio_input_port->getValue() : 0.0;
        float cutoff_input_voltage = cutoff_input_port->isConnected() ? cutoff_input_port->getValue() : getParameter("cutoff");
        float resonance_input_voltage = resonance_input_port->isConnected() ? resonance_input_port->getValue() : getParameter("resonance");
        float morph_input_voltage = morph_input_port->isConnected() ? morph_input_port->getValue() : getParameter("morph");

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
        audio_output_port->setValue(output);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "INPUT_PORT")
        {
            return audio_input_port;
        }
        else if (port_name == "CUTOFF_INPUT_PORT")
        {
            return cutoff_input_port;
        }
        else if (port_name == "RESONANCE_INPUT_PORT")
        {
            return resonance_input_port;
        }
        else if (port_name == "MORPH_INPUT_PORT")
        {
            return morph_input_port;
        }
        else if (port_name == "OUTPUT_PORT")
        {
            return audio_output_port;
        }   
        else
        {
            return nullptr;
        }
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return {audio_output_port};
    }

    std::vector<Sport *> getInputPorts() override
    {
        return {audio_input_port, cutoff_input_port, resonance_input_port, morph_input_port};
    }    
};
