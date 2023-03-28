#pragma once
#include <string> 
#include <cmath>
#include <algorithm>
#include "../BaseModule.hpp"
#include "../dsp/SchmittTrigger.hpp"

class ADSRModule : public BaseModule {

private:
    enum class State {
        IDLE,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE,
    };

    State state = State::IDLE;

    float attack_time = 0.0f;
    float decay_time = 0.0f;
    float sustain_level = 0.0f;
    float release_time = 0.0f;
    float output = 0.0f;
    float phase = 0.0f;

    SchmittTrigger trigger_input_schmitt_trigger;
    SchmittTrigger gate_input_schmitt_trigger;

public:

    Sport *trigger_input_port = new Sport(this);
    Sport *gate_input_port = new Sport(this);
    Sport *attack_input_port = new Sport(this);
    Sport *decay_input_port = new Sport(this);
    Sport *sustain_input_port = new Sport(this);
    Sport *release_input_port = new Sport(this);
    Sport *output_port = new Sport(this);

    ADSRModule() 
    {
        setParameter("attack_time", 0.1f);
        setParameter("decay_time", 0.2f);
        setParameter("sustain_level", 1.0f);
        setParameter("release_time", 0.3f);
    }

    void process(unsigned int sample_rate) override 
    {
        // float trigger = trigger_input_port->getValue();
        float gate = gate_input_port->getValue();

        // bool trigger_edge = trigger_input_schmitt_trigger.process(trigger);
        bool gate_edge = gate_input_schmitt_trigger.process(gate);

        // If gate transitios from low to high, start the attack state
        if (gate_edge)
        {
            state = State::ATTACK;
            phase = 0.0f;
        }
        // Check if the trigger is negative and the module is not already
        // in the release state. If so, start the release state.
        else if ((!gate) && (state != State::RELEASE) && (state != State::IDLE))
        {
            state = State::RELEASE;
            phase = 0.0f;
        }

        float attack_time_voltage = attack_input_port->isConnected() ? attack_input_port->getValue() : getParameter("attack_time");
        float decay_time_voltage = decay_input_port->isConnected() ? decay_input_port->getValue() : getParameter("decay_time");
        float sustain_level_voltage = sustain_input_port->isConnected() ? sustain_input_port->getValue() : getParameter("sustain_level");
        float release_time_voltage = release_input_port->isConnected() ? release_input_port->getValue() : getParameter("release_time");

        attack_time = map(attack_time_voltage, 0.0f, 10.0f, 0.01f, 2.0f);
        decay_time = map(decay_time_voltage, 0.0f, 10.0f, 0.01f, 2.0f);
        sustain_level = map(sustain_level_voltage, 0.0f, 10.0f, 0.0f, 1.0f);
        release_time = map(release_time_voltage, 0.0f, 10.0f, 0.01f, 2.0f);

        // Calculate the output based on the current state
        switch (state)
        {
            case State::IDLE:
                output = 0.0f;
                break;
            case State::ATTACK:
                output = getEnvelopeValue(phase, 0.0f, 1.0f, attack_time);
                if (phase >= 1.0f) state = State::DECAY;
                break;
            case State::DECAY:
                output = getEnvelopeValue(phase, 1.0f, sustain_level, decay_time);
                if (phase >= 1.0f) state = State::SUSTAIN;
                break;
            case State::SUSTAIN:
                output = sustain_level;
                break;
            case State::RELEASE:
                output = getEnvelopeValue(phase, sustain_level, 0.0f, release_time);
                if (phase >= 1.0f) state = State::IDLE;
                break;
            default:
                break;
        }

        // Increment the phase
        phase += 1.0f / static_cast<float>(sample_rate * 5.0f);

        // Ensure that the phase stays between 0 and 1
        if (phase >= 1.0f) phase = 1.0f;

        // Set output value, which will also alert any connected ports
        output_port->setValue(output * 10.0f);
    }


    float getEnvelopeValue(float phase, float start_value, float end_value, float time)
    {
        // Calculate the current phase of the envelope as a fraction of the total time
        float current_phase = std::min(phase / time, 1.0f);

        // Calculate the current value of the envelope as a linear interpolation between
        // the start and end values based on the current phase
        float current_value = start_value + (end_value - start_value) * current_phase;

        // Return the current value of the envelope
        return current_value;
    }

    Sport *getPortByName(std::string port_name) override
    {
        if (port_name == "TRIGGER_INPUT_PORT")
        {
            return trigger_input_port;
        }
        else if (port_name == "GATE_INPUT_PORT")
        {
            return gate_input_port;
        }
        else if (port_name == "ATTACK_INPUT_PORT")
        {
            return attack_input_port;
        }
        else if (port_name == "DECAY_INPUT_PORT")
        {
            return decay_input_port;
        }
        else if (port_name == "SUSTAIN_INPUT_PORT")
        {
            return sustain_input_port;
        }
        else if (port_name == "RELEASE_INPUT_PORT")
        {
            return release_input_port;
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
        return {
            trigger_input_port,
            gate_input_port,
            attack_input_port,
            decay_input_port,
            sustain_input_port,
            release_input_port
        };
    }

};