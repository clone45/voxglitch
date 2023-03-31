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
    int wavetable_index = 0;
    int wavetable_size = 512;
    bool wavetables_loaded = false;

public:

    Sport *frequency_input_port = new Sport(this);
    Sport *waveform_input_port = new Sport(this);
    Sport *output_port = new Sport(this);

    float* wavetables[NUM_WAVETABLES];

    // Allocate a single array to hold all the wavetable data
    float* data = new float[NUM_WAVETABLES * WAVETABLE_SIZE];

    WavetableOscillatorModule() 
    {
        setParameter("waveform", 0.0f);
        setParameter("frequency", 440.0f);

        // Read the wavetables from a file
        std::string wavetable_filename = asset::plugin(pluginInstance, "res/inner/wavetables.txt");
        
        if(readWavetables(wavetable_filename, data))
        {
            // Create an rray of pointers to the beginning of each wavetable
            for (int i = 0; i < NUM_WAVETABLES; ++i) {
                wavetables[i] = &data[i * WAVETABLE_SIZE];
            }

            wavetables_loaded = true;
        }
        
    }

    ~WavetableOscillatorModule() 
    {
        delete[] data;
    }

    void process(unsigned int sample_rate) override 
    {
        if (!wavetables_loaded) return;

        float frequency_voltage = frequency_input_port->isConnected() ? frequency_input_port->getValue() : getParameter("frequency");
        float waveform_voltage = waveform_input_port->getValue() ? waveform_input_port->getValue() : getParameter("waveform");

        // Convert the voltage value to frequency in Hz using the 1V/octave standard
        frequency = 261.625565 * powf(2.0f, frequency_voltage - 4.0f);

        // Calculate the phase increment based on the frequency
        float phase_increment = frequency / static_cast<float>(sample_rate);

        // Calculate the waveform index based on the waveform voltage
        wavetable_index = (waveform_voltage / 10.0f) * float(NUM_WAVETABLES + 1.0); // 0 to 8
        wavetable_index = clamp(wavetable_index, 0, NUM_WAVETABLES);

        // Calculate the output value using linear interpolation
        int index1 = static_cast<int>(std::floor(phase * wavetable_size));
        int index2 = static_cast<int>(std::ceil(phase * wavetable_size)) % wavetable_size;
        float frac = phase * wavetable_size - index1;

        output = lerp(wavetables[wavetable_index][index1], wavetables[wavetable_index][index2], frac);


        // Output will range from 0 to 256, which is way too loud.  we need to
        // get it in the range of -5 to 5 volts:
        output = (output / 25.6f) - 5.0f;

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