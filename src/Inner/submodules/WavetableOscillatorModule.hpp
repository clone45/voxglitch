#pragma once
#include <string>
#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"
#include "../dsp/SchmittTrigger.hpp"

// This is an example array of wavetables. Each wavetable contains 256 samples.
// You would need to replace this with your own array of actual wavetables.
#include "../res/wavetables.hpp"

class WavetableOscillatorModule : public BaseModule {

private:
    float frequency = 0.0f;
    float phase = 0.0f;
    float output = 0.0f;
    int waveform_index = 0;
    int waveform_size = 256;

public:

    Sport *frequency_input_port = new Sport(this);
    Sport *waveform_input_port = new Sport(this);
    Sport *output_port = new Sport(this);

    WavetableOscillatorModule() 
    {
        setParameter("frequency", 440.0f);
    }

    void process(unsigned int sample_rate) override 
    {
        float frequency_voltage = frequency_input_port->isConnected() ? frequency_input_port->getValue() : getParameter("frequency");

        // Convert the voltage value to frequency in Hz using the 1V/octave standard
        frequency = 261.625565 * powf(2.0f, frequency_voltage - 4.0f);

        // Calculate the phase increment based on the frequency
        float phase_increment = frequency / static_cast<float>(sample_rate);

        // If the waveform input port is connected, read the waveform index from the input.
        // Otherwise, use the waveform index stored in the module.
        if (waveform_input_port->isConnected()) 
        {
            waveform_index = static_cast<int>(std::round(waveform_input_port->getValue() * 7.0f));
            waveform_index = clamp(waveform_index, 0, 7);
        }

        // Calculate the output value by reading the appropriate sample from the current waveform.
        // The phase is incremented by the phase increment for each sample.
        output = wavetables[waveform_index][static_cast<int>(std::round(phase * waveform_size))];

        // Increment the phase
        phase += phase_increment;

        // Ensure that the phase stays between 0 and 1
        if (phase >= 1.0f) phase -= 1.0f;

        // Set output value, which will also alert any connected ports
        output_port->setValue(output);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "FREQUENCY_INPUT_PORT")
        {
            return frequency_input_port;
        }
        else if (port_name == "WAVEFORM_INPUT_PORT")
        {
            return waveform_input_port;
        }
        else if (port_name == "OUTPUT_PORT")
        {
            return output_port;
        }
        else
        {
            return nullptr;
        }
    }

    std::vector<Sport *> getOutputPorts() override
    {
        return {output_port};
    }

    std::vector<Sport *> getInputPorts() override
    {
        return {frequency_input_port, waveform_input_port};
    }
};