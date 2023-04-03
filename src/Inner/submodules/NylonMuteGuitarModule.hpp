#pragma once

#include <cmath>
#include "../BaseModule.hpp"
#include "../dsp/Delay.hpp"
#include "../dsp/OnePoleFilter.hpp"

class NylonMuteGuitarModule : public BaseModule {
private:
    float sample_rate = 44100.0f; // Set a default sample rate; this should be updated in the process method
    float max_delay_time_seconds = 0.1f; // Maximum delay time in seconds; set this according to your requirements
    int max_delay_samples = static_cast<int>(max_delay_time_seconds * sample_rate);
    float delay_time = 0.001f;
    float delay_feedback = 0.5f;
    float filter_cutoff = 2000.0f;
    float filter_resonance = 0.5f;
    float stiffness = 0.5f;
    float mass = 0.5f;
    float decay = 0.5f;

    Delay delay{max_delay_samples}; // Pass the maximum delay time in samples
    OnePoleFilter filter{filter_cutoff, sample_rate}; // Pass the initial cutoff frequency and sample rate


public:
    Sport *audio_input_port = new Sport(this);
    Sport *delay_time_input_port = new Sport(this);
    Sport *delay_feedback_input_port = new Sport(this);
    Sport *filter_cutoff_input_port = new Sport(this);
    Sport *filter_resonance_input_port = new Sport(this);
    Sport *stiffness_input_port = new Sport(this);
    Sport *mass_input_port = new Sport(this);
    Sport *decay_input_port = new Sport(this);
    Sport *output_port = new Sport(this);

    NylonMuteGuitarModule() {
        setParameter("delay_time", 0.001f);
        setParameter("delay_feedback", 0.5f);
        setParameter("filter_cutoff", 2000.0f);
        setParameter("filter_resonance", 0.5f);
        setParameter("stiffness", 0.5f);
        setParameter("mass", 0.5f);
        setParameter("decay", 0.5f);
    }

    void process(unsigned int sample_rate) override {
        this->sample_rate = sample_rate;

        // Get input values
        float input = audio_input_port->getValue();
        float delay_time_voltage = delay_time_input_port->isConnected() ? delay_time_input_port->getValue() : getParameter("delay_time");
        float delay_feedback_voltage = delay_feedback_input_port->isConnected() ? delay_feedback_input_port->getValue() : getParameter("delay_feedback");
        float filter_cutoff_voltage = filter_cutoff_input_port->isConnected() ? filter_cutoff_input_port->getValue() : getParameter("filter_cutoff");
        float filter_resonance_voltage = filter_resonance_input_port->isConnected() ? filter_resonance_input_port->getValue() : getParameter("filter_resonance");
        float stiffness_voltage = stiffness_input_port->isConnected() ? stiffness_input_port->getValue() : getParameter("stiffness");
        float mass_voltage = mass_input_port->isConnected() ? mass_input_port->getValue() : getParameter("mass");
        float decay_voltage = decay_input_port->isConnected() ? decay_input_port->getValue() : getParameter("decay");

        // Scale input voltages to desired ranges
        delay_time = map(delay_time_voltage, 0.0f, 10.0f, 0.0005f, 0.05f);
        delay_feedback = map(delay_feedback_voltage, 0.0f, 10.0f, 0.0f, 1.0f);
        filter_cutoff = map(filter_cutoff_voltage, 0.0f, 10.0f, 500.0f, 5000.0f);
        filter_resonance = map(filter_resonance_voltage, 0.0f, 10.0f, 0.1f, 0.9f);
        stiffness = map(stiffness_voltage, 0.0f, 10.0f, 0.1f, 0.9f);
        mass = map(mass_voltage, 0.0f, 10.0f, 0.1f, 0.9f);
        decay = map(decay_voltage, 0.0f, 10.0f, 0.1f, 0.9f);

        // Calculate plucking force based on stiffness and mass
        float force = stiffness * input * mass;

        // Calculate displacement and velocity of string over time
        float displacement = delay.process(delay_time * sample_rate) * decay;
        float velocity = (delay.process(0.0f) - delay.process(delay_time * sample_rate)) / (delay_time * sample_rate);

        // Calculate acceleration based on force, velocity, stiffness, and mass
        float acceleration = (force - stiffness * displacement - mass * velocity) / mass;

        // Apply the calculated acceleration to the delay line
        float delayed_sample = delay.process(delay_time * sample_rate);
        delay.set_delay_samples(delay_time * sample_rate);
        float output = delay.process(acceleration + delayed_sample * filter_resonance);

        // Apply a low-pass filter to the output of the delay line
        filter.setCoefficients(filter_cutoff, sample_rate); // Use the setCoefficients method with the cutoff frequency and sample rate
        output = filter.process(output * delay_feedback);
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "AUDIO_INPUT") return audio_input_port;
        if (port_name == "DELAY_TIME_INPUT") return delay_time_input_port;
        if (port_name == "DELAY_FEEDBACK_INPUT") return delay_feedback_input_port;
        if (port_name == "FILTER_CUTOFF_INPUT") return filter_cutoff_input_port;
        if (port_name == "FILTER_RESONANCE_INPUT") return filter_resonance_input_port;
        if (port_name == "STIFFNESS_INPUT") return stiffness_input_port;
        if (port_name == "MASS_INPUT") return mass_input_port;
        if (port_name == "DECAY_INPUT") return decay_input_port;
        if (port_name == "OUTPUT") return output_port;

        return nullptr; // Return nullptr if no port is found with the given name
    }

    std::vector<Sport *> getOutputPorts() override
    {
        std::vector<Sport *> output_ports = {output_port};
        return output_ports;
    }

    std::vector<Sport *> getInputPorts() override
    {
        std::vector<Sport *> input_ports = {
            audio_input_port, delay_time_input_port, delay_feedback_input_port,
            filter_cutoff_input_port, filter_resonance_input_port, stiffness_input_port,
            mass_input_port, decay_input_port
        };
        return input_ports;
    }

};
