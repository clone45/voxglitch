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
    int waveform_size = 512;

public:

    Sport *frequency_input_port = new Sport(this);
    Sport *waveform_input_port = new Sport(this);
    Sport *output_port = new Sport(this);

    WavetableOscillatorModule() 
    {
        setParameter("waveform", 0.0f);
        setParameter("frequency", 440.0f);
    }

    void process(unsigned int sample_rate) override 
    {
        float frequency_voltage = frequency_input_port->isConnected() ? frequency_input_port->getValue() : getParameter("frequency");
        float waveform_voltage = waveform_input_port->getValue() ? waveform_input_port->getValue() : getParameter("waveform");

        // Convert the voltage value to frequency in Hz using the 1V/octave standard
        frequency = 261.625565 * powf(2.0f, frequency_voltage - 4.0f);

        // Calculate the phase increment based on the frequency
        float phase_increment = frequency / static_cast<float>(sample_rate);

        // Calculate the waveform index based on the waveform voltage
        waveform_index = (waveform_voltage / 10.0f) * 8.0f; // 0 to 7
        waveform_index = clamp(waveform_index, 0, 7);

        // Calculate the output value using linear interpolation
        int index1 = static_cast<int>(std::floor(phase * waveform_size));
        int index2 = static_cast<int>(std::ceil(phase * waveform_size)) % waveform_size;
        float frac = phase * waveform_size - index1;
        output = lerp(wavetables[waveform_index][index1], wavetables[waveform_index][index2], frac);

        // Output will range from 0 to 256, which is way too loud.  we need to
        // get it in the range of -5 to 5 volts:
        output = (output / 256.0f) * 10.0f - 5.0f;

        // Increment the phase
        phase += phase_increment;

        // Ensure that the phase stays between 0 and 1
        if (phase >= 1.0f) phase -= 1.0f;

        // Set output value, which will also alert any connected ports
        output_port->setValue(output);
    }

    float lerp(float x1, float x2, float frac)
    {
        return x1 * (1.0f - frac) + x2 * frac;
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