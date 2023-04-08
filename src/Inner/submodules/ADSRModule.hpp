#pragma once
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

    enum INPUTS {
        TRIGGER,
        GATE,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        ATTACK_TIME_PARAM,
        DECAY_TIME_PARAM,
        SUSTAIN_LEVEL_PARAM,
        RELEASE_TIME_PARAM,
        NUM_PARAMS
    };

    ADSRModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        params[ATTACK_TIME_PARAM]->setValue(0.1f);
        params[DECAY_TIME_PARAM]->setValue(0.2f);
        params[SUSTAIN_LEVEL_PARAM]->setValue(1.0f);
        params[RELEASE_TIME_PARAM]->setValue(0.3f);
    }

    void process(unsigned int sample_rate) override 
    {
        float gate = inputs[GATE]->getVoltage();

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

        float attack_time_voltage = inputs[ATTACK]->isConnected() ? inputs[ATTACK]->getVoltage() : params[ATTACK_TIME_PARAM]->getValue();
        float decay_time_voltage = inputs[DECAY]->isConnected() ? inputs[DECAY]->getVoltage() : params[DECAY_TIME_PARAM]->getValue();
        float sustain_level_voltage = inputs[SUSTAIN]->isConnected() ? inputs[SUSTAIN]->getVoltage() : params[SUSTAIN_LEVEL_PARAM]->getValue();
        float release_time_voltage = inputs[RELEASE]->isConnected() ? inputs[RELEASE]->getVoltage() : params[RELEASE_TIME_PARAM]->getValue();

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
        outputs[OUTPUT]->setVoltage(output * 10.0f);
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
};