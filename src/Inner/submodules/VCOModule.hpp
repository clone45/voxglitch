#pragma once
#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"

// TODO:
//  - add ability to set pulse width
//  - add ability to set phase shift
//  - add ability to set sync division
//  - consider inheriting from a "oscillator" base class
//  (add mixer module)

class VCOModule : public BaseModule {

public:

    float phase = 0.0f;
    const float C4_FREQUENCY = 261.6256f;
    float previous_sync_input = 0.0f;

    enum class Waveform {
        SINE,
        TRIANGLE,
        PULSE,
        SAWTOOTH,
        NUM_WAVEFORMS
    };

    enum INPUTS {
        FREQUENCY,  
        WAVEFORM,
        SYNC_THRESHOLD,
        SYNC,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        FREQUENCY_PARAM,
        WAVEFORM_PARAM,
        SYNC_THRESHOLD_PARAM,
        PULSE_WIDTH_PARAM,
        NUM_PARAMS
    };

    VCOModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        // Set default values for parameters
        params[FREQUENCY_PARAM]->setValue(1.0f);
        params[WAVEFORM_PARAM]->setValue(4.0f);
        params[SYNC_THRESHOLD_PARAM]->setValue(5.0f);
        params[PULSE_WIDTH_PARAM]->setValue(0.5f);
    }

    void process(unsigned int sample_rate) override 
    {
        // Get the input voltages
        float frequency_voltage = inputs[FREQUENCY]->isConnected() ? inputs[FREQUENCY]->getVoltage() : params[FREQUENCY_PARAM]->getValue();
        float waveform_voltage = inputs[WAVEFORM]->isConnected() ? inputs[WAVEFORM]->getVoltage() : params[WAVEFORM_PARAM]->getValue();
        float sync_threshold = inputs[SYNC_THRESHOLD]->isConnected() ? inputs[SYNC_THRESHOLD]->getVoltage() : params[SYNC_THRESHOLD_PARAM]->getValue();
        float sync_input = inputs[SYNC]->isConnected() ? inputs[SYNC]->getVoltage() : 0.0f;

        // Get the selected waveform based on the waveform voltage
        Waveform selected_waveform = getSelectedWaveform(waveform_voltage);

        // If the sync input is connected, process it.  This will reset the phase
        // when the sync_input crosses the sync_threshold.
        processSyncInput(sync_input, sync_threshold);

        // Calculate the phase increment and update the phase
        float phase_increment = getPhaseIncrement(frequency_voltage, sample_rate);
        phase += phase_increment;

        // Ensure that the phase stays between 0 and 1
        if (phase >= 1.0f) phase -= 1.0f;

        // Calculate the output based on the selected waveform
        float out = computeWaveform(selected_waveform, phase);

        // Set output value, which will also alert any connected ports
        outputs[OUTPUT]->setVoltage(out * 5.0f);
    }

    void processSyncInput(float sync_input, float sync_threshold)
    {
        if (inputs[SYNC]->isConnected())
        {
            // The sync_threshold input port is a voltage that ranges from 0V to 10V.
            // However, we want to be able to compare the sync input to the sync threshold.
            // Since the sync input ranges from -5V to +5V, we need to convert the sync threshold
            // to a value between -5V and +5V. We do this by subtracting 5V from the sync threshold.
            sync_threshold -= 5.0f;

            // If the sync input port has a rising edge (i.e. changes from a value less than 
            // the sync threshold to a value greater than or equal to the sync threshold),
            // reset the phase to 0.
            if ((sync_input >= sync_threshold) && (previous_sync_input < sync_threshold)) 
            {
                phase = 0.0f;
            }

            // Update previous sync input value
            previous_sync_input = sync_input;
        }
    }

    float computeWaveform(Waveform waveform, float phase)
    {
        float out = 0.0f;

        switch (waveform)
        {
            case Waveform::SINE:
                out = sinf(phase * 2.0f * 3.14159265358979323846264338327950288);
                break;
            case Waveform::TRIANGLE:
                out = (phase < 0.5f) ? (-1.0f + 4.0f * phase) : (3.0f - 4.0f * phase);
                break;
            case Waveform::PULSE:
                out = (phase < this->params[PULSE_WIDTH_PARAM]->getValue()) ? 1.0f : -1.0f;
                break;
            case Waveform::SAWTOOTH:
                out = -1.0f + 2.0f * phase;
                break;
            default:
                break;
        }

        return out;
    }

    Waveform getSelectedWaveform(float waveform_voltage)
    {
        // The voltage range for selecting a waveform will range from 0v to 10v.
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
};