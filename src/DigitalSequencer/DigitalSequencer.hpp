// Look for: TODO: Left off here

struct DigitalSequencer : VoxglitchModule
{
    dsp::SchmittTrigger stepTrigger;
    dsp::SchmittTrigger sequencer_step_triggers[NUMBER_OF_SEQUENCERS];
    dsp::SchmittTrigger resetTrigger;
    dsp::SchmittTrigger sequencer_button_triggers[NUMBER_OF_SEQUENCERS];

    long clock_ignore_on_reset = 0;
    bool legacy_reset = false;
    bool first_step = true;
    bool frozen = false;
    bool frozen_trigger_gate = false;

    unsigned int tooltip_timer = 0;

    std::vector<VGSequencerPlugin> vg_sequencer_plugins;
    VGSequencerPlugin *selected_vg_sequencer_plugin;

    // I may be able to remove these two pointers once I get the vg_sequencer_plugins
    // work completed.
    VoltageSequencer *selected_voltage_sequencer;
    GateSequencer *selected_gate_sequencer;

    // VoltageSequencer voltage_sequencers[NUMBER_OF_SEQUENCERS];
    // std::vector<VoltageSequencer> voltage_sequencers;
    // VoltageSequencer *selected_voltage_sequencer;

    // GateSequencer gate_sequencers[NUMBER_OF_SEQUENCERS];
    // std::vector<GateSequencer> gate_sequencers;
    // GateSequencer *selected_gate_sequencer;

    unsigned int selected_sequencer_index = 0;
    int voltage_outputs[NUMBER_OF_SEQUENCERS];
    int gate_outputs[NUMBER_OF_SEQUENCERS];
    int sequencer_step_inputs[NUMBER_OF_SEQUENCERS];

    dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_SEQUENCERS];
    double sample_rate;

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
        SEQUENCE_SELECTION_KNOB,
        ENUMS(SEQUENCER_LENGTH_KNOBS, NUMBER_OF_SEQUENCERS),

        SEQUENCE_START_KNOB,
        ENUMS(SEQUENCER_SELECTION_BUTTONS, NUMBER_OF_SEQUENCERS),
        FREEZE_TOGGLE,

        NUM_PARAMS
    };
    enum InputIds
    {
        CLOCK_INPUT,
        STEP_INPUT,
        RESET_INPUT,
        SEQUENCER_1_STEP_INPUT,
        SEQUENCER_2_STEP_INPUT,
        SEQUENCER_3_STEP_INPUT,
        SEQUENCER_4_STEP_INPUT,
        SEQUENCER_5_STEP_INPUT,
        SEQUENCER_6_STEP_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        CLOCK_OUTPUT,
        END_OUTPUT,

        SEQ1_CV_OUTPUT,
        SEQ2_CV_OUTPUT,
        SEQ3_CV_OUTPUT,
        SEQ4_CV_OUTPUT,
        SEQ5_CV_OUTPUT,
        SEQ6_CV_OUTPUT,

        SEQ1_GATE_OUTPUT,
        SEQ2_GATE_OUTPUT,
        SEQ3_GATE_OUTPUT,
        SEQ4_GATE_OUTPUT,
        SEQ5_GATE_OUTPUT,
        SEQ6_GATE_OUTPUT,

        NUM_OUTPUTS
    };
    enum LightIds
    {
        SEQUENCER_1_LIGHT,
        SEQUENCER_2_LIGHT,
        SEQUENCER_3_LIGHT,
        SEQUENCER_4_LIGHT,
        SEQUENCER_5_LIGHT,
        SEQUENCER_6_LIGHT,
        NUM_LIGHTS
    };

    //
    // Constructor
    //
    DigitalSequencer()
    {
        // voltage_sequencers.resize(NUMBER_OF_SEQUENCERS);
        // gate_sequencers.resize(NUMBER_OF_SEQUENCERS);
        vg_sequencer_plugins.resize(NUMBER_OF_SEQUENCERS);

        voltage_outputs[0] = SEQ1_CV_OUTPUT;
        voltage_outputs[1] = SEQ2_CV_OUTPUT;
        voltage_outputs[2] = SEQ3_CV_OUTPUT;
        voltage_outputs[3] = SEQ4_CV_OUTPUT;
        voltage_outputs[4] = SEQ5_CV_OUTPUT;
        voltage_outputs[5] = SEQ6_CV_OUTPUT;

        gate_outputs[0] = SEQ1_GATE_OUTPUT;
        gate_outputs[1] = SEQ2_GATE_OUTPUT;
        gate_outputs[2] = SEQ3_GATE_OUTPUT;
        gate_outputs[3] = SEQ4_GATE_OUTPUT;
        gate_outputs[4] = SEQ5_GATE_OUTPUT;
        gate_outputs[5] = SEQ6_GATE_OUTPUT;

        sequencer_step_inputs[0] = SEQUENCER_1_STEP_INPUT;
        sequencer_step_inputs[1] = SEQUENCER_2_STEP_INPUT;
        sequencer_step_inputs[2] = SEQUENCER_3_STEP_INPUT;
        sequencer_step_inputs[3] = SEQUENCER_4_STEP_INPUT;
        sequencer_step_inputs[4] = SEQUENCER_5_STEP_INPUT;
        sequencer_step_inputs[5] = SEQUENCER_6_STEP_INPUT;

        // selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
        // selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];
        selected_vg_sequencer_plugin = &vg_sequencer_plugins[selected_sequencer_index];
        selected_voltage_sequencer = &selected_vg_sequencer_plugin->voltage_sequencer;
        selected_gate_sequencer = &selected_vg_sequencer_plugin->gate_sequencer;

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            // This will set the size of the voltage sequencer, as well as assign zeros to each element.
            /*
            voltage_sequencers[i].assign(MAX_SEQUENCER_STEPS, 0.0);
            voltage_sequencers[i].setMaxLength(MAX_SEQUENCER_STEPS);
            voltage_sequencers[i].setWindowEnd(MAX_SEQUENCER_STEPS - 1);

            gate_sequencers[i].assign(MAX_SEQUENCER_STEPS, 0.0);
            gate_sequencers[i].setMaxLength(MAX_SEQUENCER_STEPS);
            gate_sequencers[i].setWindowEnd(MAX_SEQUENCER_STEPS - 1);
            */

            vg_sequencer_plugins[i].assign(MAX_SEQUENCER_STEPS, 0.0);
            vg_sequencer_plugins[i].setMaxLength(MAX_SEQUENCER_STEPS);
            vg_sequencer_plugins[i].setWindowEnd(MAX_SEQUENCER_STEPS - 1);

            configParam(SEQUENCER_SELECTION_BUTTONS + i, 0.f, 1.f, 0.f, "Sequencer Selection Button #" + std::to_string(i));
            configParam(SEQUENCER_LENGTH_KNOBS + i, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequence Length Knob #" + std::to_string(i));
            getParamQuantity(SEQUENCER_LENGTH_KNOBS + i)->resetEnabled = false;
            getParamQuantity(SEQUENCER_LENGTH_KNOBS + i)->randomizeEnabled = false;
        }

        configInput(STEP_INPUT, "Step");
        configInput(RESET_INPUT, "Reset");

        // On boot, I seem to be getting some weird gate signals.  This keeps those
        // from triggering an output pulse when the module first loads.
        clock_ignore_on_reset = (long)(44100 / 100);
    }

    void onRandomize() override
    {
        for (int sequencer_number = 0; sequencer_number < NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            // this->voltage_sequencers[sequencer_number].randomize();
            // this->gate_sequencers[sequencer_number].randomize();
            vg_sequencer_plugins[sequencer_number].randomize();
        }
    }

    /*
    void copy(unsigned int src_sequencer_index, unsigned int dst_sequencer_index)
    {
      // src_sequencer_index = clamp(src_sequencer_index, 0, NUMBER_OF_SEQUENCERS);
      // dst_sequencer_index = clamp(dst_sequencer_index, 0, NUMBER_OF_SEQUENCERS);

      for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
      {
        this->voltage_sequencers[dst_sequencer_index].setValue(i,this->voltage_sequencers[src_sequencer_index].getValue(i));
        this->gate_sequencers[dst_sequencer_index].setValue(i,this->gate_sequencers[src_sequencer_index].getValue(i));
      }
    }
    */

    void setLengthKnobPosition(unsigned int value)
    {
        params[SEQUENCER_LENGTH_KNOBS + selected_sequencer_index].setValue(value);
    }

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
        
        // Save the voltage sequencers
        // json_object_set_new(json_root, "voltage_sequencers", SEQUENCERS::serialize(this->voltage_sequencers));
        // json_object_set_new(json_root, "gate_sequencers", SEQUENCERS::serialize(this->gate_sequencers));

        // Save Legacy Reset mode
        json_object_set_new(json_root, "legacy_reset", json_boolean(legacy_reset));

        return json_root;
    }

    void dataFromJson(json_t *json_root) override
    {
        // Load the voltage sequencers
        // SEQUENCERS::deserialize(this->voltage_sequencers, json_object_get(json_root, "voltage_sequencers"));
        // SEQUENCERS::deserialize(this->gate_sequencers, json_object_get(json_root, "gate_sequencers"));
        
        // Load Legacy Reset mode
        legacy_reset = JSON::getBoolean(json_root, "legacy_reset");
    }

    /*

    ______
    | ___ \
    | |_/ / __ ___   ___ ___  ___ ___
    |  __/ '__/ _ \ / __/ _ \/ __/ __|  ====================================================
    | |  | | | (_) | (_|  __/\__ \__ \  ====================================================
    \_|  |_|  \___/ \___\___||___/___/  ====================================================


    */

    void process(const ProcessArgs &args) override
    {
        bool trigger_output_pulse = false;
        this->sample_rate = args.sampleRate;

        //
        // See if someone pressed one of the sequence selection buttons
        //
        // If any of the sequence buttons were pressed, set the index "selected_sequencer_index"
        // which will be used to look up the selected voltage and gate sequencers from
        // the voltage_sequencers[] and gate_sequencers[] arrays

        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            if (sequencer_button_triggers[i].process(params[SEQUENCER_SELECTION_BUTTONS + i].getValue()))
            {
                selected_sequencer_index = i;
            }
        }

        // selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
        // selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];
        selected_vg_sequencer_plugin = &vg_sequencer_plugins[selected_sequencer_index];
        selected_voltage_sequencer = &selected_vg_sequencer_plugin->voltage_sequencer;
        selected_gate_sequencer = &selected_vg_sequencer_plugin->gate_sequencer;

        // Highlight only selected sequence buttton
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            params[SEQUENCER_SELECTION_BUTTONS + i].setValue(selected_sequencer_index == i);
        }

        //
        // Set all of the sequence lengths by checking the corresponding knobs
        //
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            // voltage_sequencers[i].setLength(clamp((int)params[SEQUENCER_LENGTH_KNOBS + i].getValue(), 1, 32));
            // gate_sequencers[i].setLength(clamp((int)params[SEQUENCER_LENGTH_KNOBS + i].getValue(), 1, 32));
            vg_sequencer_plugins[i].setLength(clamp((int)params[SEQUENCER_LENGTH_KNOBS + i].getValue(), 1, 32));
        }

        //
        // FROZEN, FROZEN, FROZEN
        //
        // This is a pretty crazy IF statement.  It's saying, "If the frozen flag
        // is FALSE, then step the sequencers"  Why would someone want to
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

                stepTrigger.reset();
                first_step = true;

                for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
                {
                    sequencer_step_triggers[i].reset();
                    // voltage_sequencers[i].reset();
                    // gate_sequencers[i].reset();
                    vg_sequencer_plugins[i].reset();
                }
            }

            //
            // The legacy reset option in the context menu tells the module to accept
            // clock signals immedately after resetting.  Reset signals are supposed to
            // cause clock signals to be ignored for 1 millisecond, however, some older
            // modules don't do that, so this flag helps with compatibility with older
            // modules.
            //
            if (legacy_reset || clock_ignore_on_reset == 0)
            {
                // Check to see if there's a pulse at the global step trigger input.
                bool global_step_trigger = stepTrigger.process(inputs[STEP_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);
                bool step;

                // reset_first_step ensures that the first step of the sequence is not skipped
                // after a reset.  This functionality is disbled in legacy reset mode.
                bool reset_first_step = false;

                for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
                {
                    step = false;

                    // This line is saying, "If there's a wire connected to the individual
                    // sequencer's step input, then it should override the global step
                    // trigger input for this specific sequencer"
                    if (inputs[sequencer_step_inputs[i]].isConnected() == false)
                    {
                        if (global_step_trigger)
                            step = true;
                    }
                    else if (sequencer_step_triggers[i].process(inputs[sequencer_step_inputs[i]].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
                    {
                        step = true;
                    }

                    if (step) // Step == true means, "step the sequencer forward one step"
                    {
                        // If legacy reset is true or it's not the first step of the sequence
                        // then go ahead and step the sequences.  Otherwise skip the first
                        // step.
                        if (legacy_reset || first_step == false)
                        {
                            // voltage_sequencers[i].step();
                            // gate_sequencers[i].step();
                            vg_sequencer_plugins[i].step();
                        }
                        else
                        {
                            reset_first_step = true;
                        }

                        // If the gate sequence is TRUE, then start the pulse generator to
                        // output the gate signal.
                        if (vg_sequencer_plugins[i].gate_sequencer.getValue())
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
                // TODO: Left off here
                /*
                if (voltage_sequencers[i].sample_and_hold && !gate_sequencers[i].getValue())
                {
                    continue; // Skip setting voltage when sample_and_hold is true and getValue() is false
                }
                */

                if(vg_sequencer_plugins[i].voltage_sequencer.sample_and_hold && !vg_sequencer_plugins[i].getGate())
                {
                    continue; // Skip setting voltage when sample_and_hold is true and getValue() is false
                }

                // float voltage = voltage_sequencers[i].getVoltage();
                float voltage = vg_sequencer_plugins[i].getVoltage();
                outputs[voltage_outputs[i]].setVoltage(voltage);
            }
        } // END IF NOT FROZEN

        else // IF FROZEN
        {
            // output values
            for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
            {
                // Notice that this ignores sample + hold.  This is the main reason
                // for duplicating this code between the frozen/not frozen IF statments.
                // outputs[voltage_outputs[i]].setVoltage(voltage_sequencers[i].getOutput());

                // float voltage = voltage_sequencers[i].getVoltage();
                float voltage = vg_sequencer_plugins[i].getVoltage();
                outputs[voltage_outputs[i]].setVoltage(voltage);
            }

            if (frozen_trigger_gate)
            {
                gateOutputPulseGenerators[selected_sequencer_index].trigger(0.01f);
                frozen_trigger_gate = false;
            }
        }

        // process trigger outputs
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
            outputs[gate_outputs[i]].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
        }

        if (clock_ignore_on_reset > 0)
            clock_ignore_on_reset--;
        if (tooltip_timer > 0)
            tooltip_timer--;
    };
};