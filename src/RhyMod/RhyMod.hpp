//
// need:
// 1. Length selection (knob/display)
// 2. Step (knob/display)
// 3. Percentage (knob/display)
// 4. Step input
// 5. reset input
// maybe a maximum length?

#include <random>

struct RhyMod : VoxglitchModule
{
    dsp::BooleanTrigger step_trigger;
    dsp::BooleanTrigger reset_trigger;
    dsp::PulseGenerator output_pulse_generator;

    float percentages[64];

    unsigned int step = 0;
    unsigned int sequence_length = 16;

    dsp::TTimer<double> reset_timer;
    bool first_step = true;
    bool wait_for_reset_timer = false;
    bool playback = true;

    // TODO: Figure this out
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        STEP_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        GATE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    RhyMod()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    //
    // Autosave module data.  VCV Rack decides when this should be called.
    //
    // My saving and loading code could be more compact by saving arrays of
    // "ball" data tidily packaged up.  I'll do that if this code ever goes
    // through another iteration.
    //

    json_t *dataToJson() override
    {
        json_t *json_root = json_object();
        return json_root;
    }

    // Load module data
    void dataFromJson(json_t *json_root) override
    {
    }

    void reset()
    {
        first_step = true;
        step = 0;
        wait_for_reset_timer = true;

        // Set up a counter so that the clock input will ignore
        // incoming clock pulses for 1 millisecond after a reset input. This
        // is to comply with VCV Rack's standards.  See section "Timing" at
        // https://vcvrack.com/manual/VoltageStandards

        reset_timer.reset();
    }

    void process(const ProcessArgs &args) override
    {
        // Process RESET input
        if (reset_trigger.process(inputs[RESET_INPUT].getVoltage()))
        {
            reset();
        }

        // Process STEP input
        if (!wait_for_reset_timer && step_trigger.process(inputs[STEP_INPUT].getVoltage()))
        {
            // If there's a step input, but first_step is true, then don't
            // increment the step and output the value at step #1
            if (first_step)
            {
                first_step = false;
            }
            // Otherwise, step the sequencer
            else
            {
                step++;

                // If we're at the end of the sequencer, wrap to the beginning
                if (step == sequence_length)
                {
                    step = 0;
                }
            }

            if (random(percentages[step]))
            {
                output_pulse_generator.trigger(0.01f);
            }
                
        }

        if (wait_for_reset_timer)
        {
            // Accumulate time in reset_timer
            if (reset_timer.process(args.sampleTime * 1000.0) > 1.0)
            {
                wait_for_reset_timer = false;
                reset_timer.reset();
            }
        }

        //
        // Outputs
        //

        bool output_pulse = output_pulse_generator.process(1.0 / args.sampleRate);
        outputs[GATE_OUTPUT].setVoltage((output_pulse ? 10.0f : 0.0f));

    }
};
