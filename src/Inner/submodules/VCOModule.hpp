#pragma once
#include <string> 
#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"

class VCOModule : public BaseModule {

private:
    float phase = 0.0f;
    const float C4_FREQUENCY = 261.6256f;

    enum class Waveform {
        SINE,
        TRIANGLE,
        PULSE,
        SAWTOOTH,
        NUM_WAVEFORMS
    };

public:

    Sport *output_port = new Sport(this);
    Sport *frequence_input_port = new Sport(this);
    Sport *waveform_input_port = new Sport(this);

    VCOModule() 
    {
        setParameter("frequency", 440.0f);
        setParameter("waveform", 4.0f);
    }

    void process(unsigned int sample_rate) override 
    {
        float frequency_voltage = frequence_input_port->isConnected() ? frequence_input_port->getValue() : getParameter("frequency");
        float waveform_voltage = waveform_input_port->isConnected() ? waveform_input_port->getValue() : getParameter("waveform");

        // Get the selected waveform based on the waveform voltage
        Waveform selected_waveform = getSelectedWaveform(waveform_voltage);

        // Calculate the phase increment and update the phase
        float phase_increment = getPhaseIncrement(frequency_voltage, sample_rate);
        phase += phase_increment;

        // Ensure that the phase stays between 0 and 1
        if (phase >= 1.0f) phase -= 1.0f;

        // Calculate the output based on the selected waveform
        float out = 0.0f;

        switch (selected_waveform)
        {
            case Waveform::SINE:
                out = sinf(phase * 2.0f * 3.14159265358979323846264338327950288);
                break;
            case Waveform::TRIANGLE:
                out = (phase < 0.5f) ? (-1.0f + 4.0f * phase) : (3.0f - 4.0f * phase);
                break;
            case Waveform::PULSE:
                out = (phase < getParameter("pulse_width")) ? 1.0f : -1.0f;
                break;
            case Waveform::SAWTOOTH:
                out = -1.0f + 2.0f * phase;
                break;
            default:
                break;
        }

        // Set output value, which will also alert any connected ports
        output_port->setValue(out * 5.0f);
    }

    Waveform getSelectedWaveform(float waveform_voltage)
    {
        // The voltage range for selecting a waveform will range from 0v 
        // to 10v.
        int waveform_selection = (waveform_voltage / 10.0f) * (int)Waveform::NUM_WAVEFORMS;
        waveform_selection = clamp(waveform_selection, 0, (int)Waveform::NUM_WAVEFORMS - 1);

        return static_cast<Waveform>(waveform_selection);
    }

    float getPhaseIncrement(float frequency_voltage, unsigned int sample_rate)
    {
        // Convert the voltage to a frequency in Hertz using the 1 V/oct standard
        float frequency = C4_FREQUENCY * powf(2.0f, frequency_voltage);

        // Calculate phase increment based on the frequency        
        return frequency / static_cast<float>(sample_rate);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "OUTPUT_PORT"){
            return(output_port);
        }
        else if (port_name == "FREQUENCY_INPUT_PORT"){
            return(frequence_input_port);
        }
        else {
           return(nullptr);
        }
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return {
            output_port
        };
    }

    std::vector<Sport *> getInputPorts() override
    {
        return {    
            frequence_input_port     
        };
    }    
};