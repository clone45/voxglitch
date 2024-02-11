//
// TODO:
//  * Add "mod" input and have it configurable

struct DigitalSequencerXP : VoxglitchModule
{
    dsp::SchmittTrigger stepTriggers[NUMBER_OF_SEQUENCERS];
    dsp::SchmittTrigger sequencer_button_triggers[NUMBER_OF_SEQUENCERS];
    dsp::SchmittTrigger resetTrigger;

    long clock_ignore_on_reset = 0;
    bool legacy_reset = false;
    bool first_step = true;
    bool frozen = false;
    bool frozen_trigger_gate = false;
    bool global_step_trigger = false;

    unsigned int tooltip_timer = 0;

    std::vector<VoltageSequencer> voltage_sequencers;
    VoltageSequencer *selected_voltage_sequencer;
    std::vector<GateSequencer> gate_sequencers;
    GateSequencer *selected_gate_sequencer;

    unsigned int selected_sequencer_index = 0;
    int voltage_outputs[NUMBER_OF_SEQUENCERS];
    int gate_outputs[NUMBER_OF_SEQUENCERS];
    int sequencer_step_inputs[NUMBER_OF_SEQUENCERS];
    bool step[NUMBER_OF_SEQUENCERS];

    dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_SEQUENCERS];
    double sample_rate;

    // There must be a better way...
    std::string labels[NUMBER_OF_SEQUENCERS] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};

    bool sequencer_button_is_triggered[NUMBER_OF_SEQUENCERS];

    std::string voltage_range_names[NUMBER_OF_VOLTAGE_RANGES] = {
        "0.0 to 10.0",
        "-10.0 to 10.0",
        "0.0 to 5.0",
        "-5.0 to 5.0",
        "0.0 to 3.0",
        "-3.0 to 3.0",
        "0.0 to 1.0",
        "-1.0 to 1.0"};

    std::string snap_division_names[NUMBER_OF_SNAP_DIVISIONS] = {"None", "8", "10", "12", "16", "24", "32", "36"};

    enum ParamIds
    {
        ENUMS(SEQUENCER_BUTTONS, NUMBER_OF_SEQUENCERS),
        FREEZE_TOGGLE,
        NUM_PARAMS
    };
    enum InputIds
    {
        POLY_STEP_INPUT,
        POLY_LENGTH_INPUT,
        RESET_INPUT,
        POLY_MOD_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        POLY_CV_OUTPUT,
        POLY_GATE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(SEQUENCER_LIGHTS, NUMBER_OF_SEQUENCERS),
        NUM_LIGHTS
    };

    //
    // Constructor
    //
    DigitalSequencerXP()
    {
        voltage_sequencers.resize(NUMBER_OF_SEQUENCERS);
        gate_sequencers.resize(NUMBER_OF_SEQUENCERS);

        selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
        selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

        // Set the size of the voltage sequencer, as well as assign zeros to each element.
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            voltage_sequencers[i].assign(MAX_SEQUENCER_STEPS, 0.0);
            voltage_sequencers[i].setMaxLength(MAX_SEQUENCER_STEPS);
            voltage_sequencers[i].setWindowEnd(MAX_SEQUENCER_STEPS - 1);

            gate_sequencers[i].assign(MAX_SEQUENCER_STEPS, 0.0);
            gate_sequencers[i].setMaxLength(MAX_SEQUENCER_STEPS);
            gate_sequencers[i].setWindowEnd(MAX_SEQUENCER_STEPS - 1);
        }

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // On boot, I seem to be getting some weird gate signals.  This keeps those
        // from triggering an output pulse when the module first loads.
        clock_ignore_on_reset = (long)(44100 / 100);
    }

    void onRandomize() override
    {
        for (int sequencer_number = 0; sequencer_number < NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            for (int i = 0; i < MAX_SEQUENCER_STEPS; i++)
            {
                this->voltage_sequencers[sequencer_number].randomize();
                this->gate_sequencers[sequencer_number].randomize();
            }
        }
    }

    /*
    void copy(unsigned int src_sequencer_index, unsigned int dst_sequencer_index)
    {
      for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
      {
        this->voltage_sequencers[dst_sequencer_index].setValue(i,this->voltage_sequencers[src_sequencer_index].getValue(i));
        this->gate_sequencers[dst_sequencer_index].setValue(i,this->gate_sequencers[src_sequencer_index].getValue(i));
      }
    }
    */

    void forceGateOut()
    {
        frozen_trigger_gate = true;
    }

    /*
    ==================================================================================================================================================
      SAVE & LOAD
    ==================================================================================================================================================
    */

    json_t *dataToJson() override
    {

        json_t *json_root = json_object();

        json_object_set_new(json_root, "voltage_sequencers", SEQUENCERS::serialize(this->voltage_sequencers));
        json_object_set_new(json_root, "gate_sequencers", SEQUENCERS::serialize(this->gate_sequencers));


        // Save Legacy Reset mode
        json_object_set_new(json_root, "legacy_reset", json_integer(legacy_reset));

        //
        // Save the labels
        //
        json_t *labels_json_array = json_array();
        for (int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            json_array_append_new(labels_json_array, json_string(labels[i].c_str()));
        }
        json_object_set(json_root, "labels", labels_json_array);
        json_decref(labels_json_array);

        return json_root;
    }

    // Autoload settings
    void dataFromJson(json_t *json_root) override
    {

        // Load the voltage sequencers
        SEQUENCERS::deserialize(this->voltage_sequencers, json_object_get(json_root, "voltage_sequencers"));
        SEQUENCERS::deserialize(this->gate_sequencers, json_object_get(json_root, "gate_sequencers"));

        //
        // Load legacy reset flag
        //
        json_t *legacy_reset_json = json_object_get(json_root, "legacy_reset");
        if (legacy_reset_json)
            legacy_reset = json_integer_value(legacy_reset_json);

        //
        // Load the labels
        //
        json_t *labels_json = json_object_get(json_root, "labels");

        if (labels_json)
        {
            size_t i;
            json_t *label_json;

            json_array_foreach(labels_json, i, label_json)
            {
                labels[i] = json_string_value(label_json);
            }
        }
    }

    /*

      ______
      | ___ \
      | |_/ / __ ___   ___ ___  ___ ___
      |  __/ '__/ _ \ / __/ _ \/ __/ __|
      | |  | | | (_) | (_|  __/\__ \__ \
      \_|  |_|  \___/ \___\___||___/___/


      */

    void process(const ProcessArgs &args) override
    {
        this->sample_rate = args.sampleRate;
        bool trigger_output_pulse = false;

        //
        // Set the selected voltage and gate sequencers
        //

        selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
        selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            if (sequencer_button_triggers[i].process(params[SEQUENCER_BUTTONS + i].getValue()))
            {
                selected_sequencer_index = i;
            }
        }

        // Highlight only selected sequence buttton
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            params[SEQUENCER_BUTTONS + i].setValue(selected_sequencer_index == i);
        }

        //
        // Process POLY step input.  The end result is that the step[] array will
        // be filled with correct step instructions for each sequencer.
        // step[] array = { false, false, true, false, false, etc... }
        poll_step_inputs();

        // Read the POLY lenth input.  In this case, we're going to set the
        // sequencer lengths immediately.  Unlike step inputs, there is no universal
        // sequencer length input if only 1 channel of the input is provided.
        set_sequencer_lengths();

        //
        // FROZEN, FROZEN, FROZEN
        //
        // This is a pretty crazy IF statement.  It's saying, "If the frozen flag
        // is FALSE, then don't step the sequencers"  Why would someone want to
        // freeze the sequencers?  Freezing the sequencers can be used when editing
        // sequences so you can hear your changes immediately.  In a traditional
        // analog sequencer, when you adjust a voltage knob, you won't hear the
        // change until the sequencer reaches that step.  That slows things down
        // because you have to wait between updates to hear the changes.  When you
        // freeze the sequencers, the step that you're editing is always sent to
        // the outputs, making it easier to fine tune values.

        if (frozen == false)
        {

            // On incoming RESET, reset the sequencers
            if (resetTrigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
            {
                // Set up a (reverse) counter so that the clock input will ignore
                // incoming clock pulses for 1 millisecond after a reset input. This
                // is to comply with VCV Rack's standards.  See section "Timing" at
                // https://vcvrack.com/manual/VoltageStandards

                clock_ignore_on_reset = (long)(args.sampleRate / 100);
                first_step = true;

                for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
                {
                    stepTriggers[i].reset();
                    voltage_sequencers[i].reset();
                    gate_sequencers[i].reset();
                }
            }

            //
            // The legacy reset option in the context menu tells the module to accept
            // clock signals immedately after resetting.  Reset signals are supposed to
            // cause clock signals to be ignored for 1 millisecond, however, some older
            // modules don't do that, so this flag helps with compatibility with older
            // modules.
            //
            // This code block gets called either immedately after a clock input, or
            // very shortly after

            if (legacy_reset || clock_ignore_on_reset == 0)
            {
                // reset_first_step ensures that the first step of the sequence is not skipped
                // after a reset.  This functionality is disbled in legacy reset mode.
                bool reset_first_step = false;

                for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
                {
                    if (step[i]) // Step == true means, "step the sequencer forward one step"
                    {
                        // If legacy reset is true or it's not the first step of the sequence
                        // then go ahead and step the sequences.  Otherwise skip the first
                        // step.
                        if (legacy_reset || first_step == false)
                        {
                            voltage_sequencers[i].step();
                            gate_sequencers[i].step();
                        }
                        else
                        {
                            reset_first_step = true;
                        }

                        // If the gate sequence is TRUE, then start the pulse generator to
                        // output the gate signal.
                        if (gate_sequencers[i].getValue())
                            gateOutputPulseGenerators[i].trigger(0.01f);
                    }
                }

                // If the first step of the sequence was skipped, don't skip it again later on!
                if (reset_first_step == true)
                    first_step = false;
            }

            // output values
            for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
            {
                if (voltage_sequencers[i].sample_and_hold)
                {
                    if (gate_sequencers[i].getValue())
                        outputs[POLY_CV_OUTPUT].setVoltage(voltage_sequencers[i].getVoltage(), i);
                }
                else
                {
                    outputs[POLY_CV_OUTPUT].setVoltage(voltage_sequencers[i].getVoltage(), i);
                }
            }
            outputs[POLY_CV_OUTPUT].setChannels(NUMBER_OF_SEQUENCERS);

        } // END IF NOT FROZEN

        else // IF FROZEN
        {
            // output values
            for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
            {
                // Notice that this ignores sample + hold.  This is the main reason
                // for duplicating this code between the frozen/not frozen IF statments.
                outputs[POLY_CV_OUTPUT].setVoltage(voltage_sequencers[i].getVoltage(), i);
            }

            if (frozen_trigger_gate)
            {
                gateOutputPulseGenerators[selected_sequencer_index].trigger(0.01f);
                frozen_trigger_gate = false;
            }
        }

        //
        // process trigger outputs
        //
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
            outputs[POLY_GATE_OUTPUT].setVoltage((trigger_output_pulse ? 10.0f : 0.0f), i);
        }
        outputs[POLY_GATE_OUTPUT].setChannels(NUMBER_OF_SEQUENCERS);

        //
        // Adjust timers
        //
        if (clock_ignore_on_reset > 0)
            clock_ignore_on_reset--;
        if (tooltip_timer > 0)
            tooltip_timer--;

        // Light up currently selected sequencer lamp
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            if (i == selected_sequencer_index)
            {
                lights[SEQUENCER_LIGHTS + i].setBrightness(1);
            }
            else
            {
                lights[SEQUENCER_LIGHTS + i].setBrightness(0);
            }
        }
    }

    void poll_step_inputs()
    {
        // inputs[POLY_STEP_INPUT].setChannels(NUMBER_OF_SEQUENCERS);
        unsigned int number_of_step_input_channels = inputs[POLY_STEP_INPUT].getChannels();

        // If polyphonic step input is being used
        if (number_of_step_input_channels > 1)
        {
            for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
            {
                step[i] = stepTriggers[i].process(inputs[POLY_STEP_INPUT].getVoltage(i), constants::gate_low_trigger, constants::gate_high_trigger);
            }
        }
        // If a monophonic (or no) input is being used
        else
        {
            // Poll the first step input channel
            bool stepped = stepTriggers[0].process(inputs[POLY_STEP_INPUT].getVoltage(0), constants::gate_low_trigger, constants::gate_high_trigger);

            // Apply the value found in the previous step to channels 2, 3, 4, etc...
            for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
                step[i] = stepped;
        }
    }

    void set_sequencer_lengths()
    {
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            if (i < (unsigned int)inputs[POLY_LENGTH_INPUT].getChannels()) // If there's an input controlling this sequencer
            {
                float length_input = inputs[POLY_LENGTH_INPUT].getVoltage(i);
                int length = ((length_input / 10.0) * 32) + 1;
                length = clamp(length, 1, 32);

                voltage_sequencers[i].setLength((int)length);
                gate_sequencers[i].setLength((int)length);
            }
        }
    }
};
