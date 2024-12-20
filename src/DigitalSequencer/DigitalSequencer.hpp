struct DigitalSequencer : VoxglitchModule
{
    dsp::SchmittTrigger stepTrigger;
    dsp::SchmittTrigger resetTrigger;
    dsp::SchmittTrigger sequencer_step_triggers[NUMBER_OF_SEQUENCERS];
    dsp::SchmittTrigger sequencer_button_triggers[NUMBER_OF_SEQUENCERS];

    long clock_ignore_on_reset = 0;
    bool legacy_reset = false;
    bool first_step = true;
    bool frozen = false;
    bool frozen_trigger_gate = false;
    unsigned int tooltip_timer = 0;

    VoltageSequencer voltage_sequencers[NUMBER_OF_SEQUENCERS];
    VoltageSequencer *selected_voltage_sequencer = &voltage_sequencers[0];

    GateSequencer gate_sequencers[NUMBER_OF_SEQUENCERS];
    GateSequencer *selected_gate_sequencer = &gate_sequencers[0];

    unsigned int selected_sequencer_index = 0;
    int voltage_outputs[NUMBER_OF_SEQUENCERS] = {};
    int gate_outputs[NUMBER_OF_SEQUENCERS] = {};
    int sequencer_step_inputs[NUMBER_OF_SEQUENCERS] = {};
    unsigned int snap_division_indexes[NUMBER_OF_SEQUENCERS] = {};
    unsigned int voltage_range_indexes[NUMBER_OF_SEQUENCERS] = {};

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

    double voltage_ranges[8][2] = {
        {0.0, 10.0},
        {-10.0, 10.0},
        {0.0, 5.0},
        {-5.0, 5.0},
        {0.0, 3.0},
        {-3.0, 3.0},
        {0.0, 1.0},
        {-1.0, 1.0}};

    std::string snap_division_names[NUMBER_OF_SNAP_DIVISIONS] = {"None", "8", "10", "12", "16", "24", "32", "36"};
    unsigned int snap_division_values[NUMBER_OF_SNAP_DIVISIONS] = {0, 8, 10, 12, 16, 24, 32, 36};

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
        ENUMS(SEQUENCER_LIGHTS, NUMBER_OF_SEQUENCERS),
        NUM_LIGHTS
    };

    //
    // Constructor
    //
    DigitalSequencer()
    {
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

        selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
        selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            // These MUST BE CALLED before the sequencers will function
            voltage_sequencers[i].assign(MAX_SEQUENCER_STEPS, 0.0);
            gate_sequencers[i].assign(MAX_SEQUENCER_STEPS, 0.0);

            voltage_sequencers[i].setMaxLength(MAX_SEQUENCER_STEPS);
            gate_sequencers[i].setMaxLength(MAX_SEQUENCER_STEPS);

            configParam(SEQUENCER_SELECTION_BUTTONS + i, 0.f, 1.f, 0.f, "Sequencer Selection Button #" + std::to_string(i));
            configParam(SEQUENCER_LENGTH_KNOBS + i, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequence Length Knob #" + std::to_string(i));
            getParamQuantity(SEQUENCER_LENGTH_KNOBS + i)->resetEnabled = false;
            getParamQuantity(SEQUENCER_LENGTH_KNOBS + i)->randomizeEnabled = false;

            // snap sequence length to 32 steps
            getParamQuantity(SEQUENCER_LENGTH_KNOBS + i)->snapEnabled = true;
            getParamQuantity(SEQUENCER_LENGTH_KNOBS + i)->smoothEnabled = false;
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
            for (int i = 0; i < MAX_SEQUENCER_STEPS; i++)
            {
                this->voltage_sequencers[sequencer_number].randomize();
                this->gate_sequencers[sequencer_number].randomize();
            }
        }
    }

    float getScaledOutput(unsigned int sequencer_index)
    {
      unsigned int voltage_range_index = this->voltage_range_indexes[sequencer_index];
      float value = this->voltage_sequencers[sequencer_index].getValue();
      float low_voltage = this->voltage_ranges[voltage_range_index][0];
      float high_voltage = this->voltage_ranges[voltage_range_index][1];
      float output = rescale(value, 0.0, 1.0, low_voltage, high_voltage);

      return output;
    }

    float getScaledOutput(unsigned int sequencer_index, unsigned int step)
    {
      unsigned int voltage_range_index = this->voltage_range_indexes[sequencer_index];
      float value = this->voltage_sequencers[sequencer_index].getValue(step);
      float low_voltage = this->voltage_ranges[voltage_range_index][0];
      float high_voltage = this->voltage_ranges[voltage_range_index][1];
      float output = rescale(value, 0.0, 1.0, low_voltage, high_voltage);

      return output;
    }

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


    // Autosave settings
    json_t *dataToJson() override
    {
        json_t *json_root = json_object();

        //
        // Save patterns
        //

        json_t *sequences_json_array = json_array();

        for (int sequencer_number = 0; sequencer_number < NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_t *pattern_json_array = json_array();

            for (int i = 0; i < MAX_SEQUENCER_STEPS; i++)
            {
                json_array_append_new(pattern_json_array, json_real(this->voltage_sequencers[sequencer_number].getValue(i)));
            }

            json_array_append_new(sequences_json_array, pattern_json_array);
        }

        json_object_set(json_root, "patterns", sequences_json_array);
        json_decref(sequences_json_array);

        //
        // Save gates
        //

        json_t *gates_json_array = json_array();

        for (int sequencer_number = 0; sequencer_number < NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_t *pattern_json_array = json_array();

            for (int i = 0; i < MAX_SEQUENCER_STEPS; i++)
            {
                json_array_append_new(pattern_json_array, json_integer(this->gate_sequencers[sequencer_number].getValue(i)));
            }

            json_array_append_new(gates_json_array, pattern_json_array);
        }

        json_object_set(json_root, "gates", gates_json_array);
        json_decref(gates_json_array);

        //
        // Save sequencer lengths
        //
        json_t *sequencer_lengths_json_array = json_array();
        for (int sequencer_number = 0; sequencer_number < NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_array_append_new(sequencer_lengths_json_array, json_integer(this->voltage_sequencers[sequencer_number].getWindowEnd()));
        }
        json_object_set(json_root, "lengths", sequencer_lengths_json_array);
        json_decref(sequencer_lengths_json_array);

        //
        // Save sequencer voltage range index selections
        //
        json_t *sequencer_voltage_range_json_array = json_array();
        for (int sequencer_number = 0; sequencer_number < NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_array_append_new(sequencer_voltage_range_json_array, json_integer(this->voltage_range_indexes[sequencer_number]));
        }
        json_object_set(json_root, "voltage_ranges", sequencer_voltage_range_json_array);
        json_decref(sequencer_voltage_range_json_array);

        //
        // Save sequencer snap index selections
        //
        json_t *sequencer_snap_json_array = json_array();
        for (int sequencer_number = 0; sequencer_number < NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_array_append_new(sequencer_snap_json_array, json_integer(this->voltage_sequencers[sequencer_number].getSnapDivision()));
        }
        json_object_set(json_root, "snap_divisions", sequencer_snap_json_array);
        json_decref(sequencer_snap_json_array);

        //
        // Save sequencer sample and hold selections
        //
        json_t *sequencer_sh_json_array = json_array();
        for (int sequencer_number = 0; sequencer_number < NUMBER_OF_SEQUENCERS; sequencer_number++)
        {
            json_array_append_new(sequencer_sh_json_array, json_integer(this->voltage_sequencers[sequencer_number].sample_and_hold));
        }
        json_object_set(json_root, "sample_and_hold", sequencer_sh_json_array);
        json_decref(sequencer_sh_json_array);

        // Save Legacy Reset mode
        json_object_set_new(json_root, "legacy_reset", json_integer(legacy_reset));

        return json_root;
    }

    // Autoload settings
    void dataFromJson(json_t *json_root) override
    {
        //
        // Load patterns
        //

        json_t *pattern_arrays_data = json_object_get(json_root, "patterns");

        if (pattern_arrays_data)
        {
            size_t pattern_number;
            json_t *json_pattern_array;

            json_array_foreach(pattern_arrays_data, pattern_number, json_pattern_array)
            {
                for (int i = 0; i < MAX_SEQUENCER_STEPS; i++)
                {
                    // Get pattern data, regardless if it's an int or real.  json_number_value
                    // will convert the result to a real.
                    double value = json_number_value(json_array_get(json_pattern_array, i));

                    // If the value is greater than 1.0, it means that an older patch is being
                    // loaded and we'll need to convert the voltage from 0 to 214
                    if (value > 1.0)
                    {
                        value = value / 214.0;
                    }

                    this->voltage_sequencers[pattern_number].setValue(i, value);
                }
            }
        }

        //
        // Load gates
        //

        json_t *gates_arrays_data = json_object_get(json_root, "gates");

        if (gates_arrays_data)
        {
            size_t pattern_number;
            json_t *json_pattern_array;

            json_array_foreach(gates_arrays_data, pattern_number, json_pattern_array)
            {
                for (int i = 0; i < MAX_SEQUENCER_STEPS; i++)
                {
                    // this->gates[pattern_number][i] = json_integer_value(json_array_get(json_pattern_array, i));
                    this->gate_sequencers[pattern_number].setValue(i, json_integer_value(json_array_get(json_pattern_array, i)));
                }
            }
        }

        //
        // Load lengths
        //
        json_t *lengths_json_array = json_object_get(json_root, "lengths");

        if (lengths_json_array)
        {
            size_t sequencer_number;
            json_t *length_json;

            json_array_foreach(lengths_json_array, sequencer_number, length_json)
            {
                this->voltage_sequencers[sequencer_number].setWindowEnd(json_integer_value(length_json));
                this->gate_sequencers[sequencer_number].setWindowEnd(json_integer_value(length_json));
            }
        }

        //
        // Load voltage ranges
        //
        json_t *voltage_ranges_json_array = json_object_get(json_root, "voltage_ranges");

        if (voltage_ranges_json_array)
        {
            size_t sequencer_number;
            json_t *voltage_range_json;

            json_array_foreach(voltage_ranges_json_array, sequencer_number, voltage_range_json)
            {
                // this->voltage_sequencers[sequencer_number].voltage_range_index = json_integer_value(voltage_range_json);
                this->voltage_range_indexes[sequencer_number] = json_integer_value(voltage_range_json);
            }
        }

        //
        // Load snap divisions
        //
        json_t *snap_divions_json_array = json_object_get(json_root, "snap_divisions");

        if (snap_divions_json_array)
        {
            size_t sequencer_number;
            json_t *snap_divion_json;

            json_array_foreach(snap_divions_json_array, sequencer_number, snap_divion_json)
            {
                voltage_sequencers[sequencer_number].setSnapDivision(json_integer_value(snap_divion_json));
            }
        }

        //
        // Load Sample and Hold settings
        //
        json_t *sh_json_array = json_object_get(json_root, "sample_and_hold");

        if (sh_json_array)
        {
            size_t sequencer_number;
            json_t *sh_json;

            json_array_foreach(sh_json_array, sequencer_number, sh_json)
            {
                this->voltage_sequencers[sequencer_number].sample_and_hold = json_integer_value(sh_json);
            }
        }

        json_t *legacy_reset_json = json_object_get(json_root, "legacy_reset");
        if (legacy_reset_json)
            legacy_reset = json_integer_value(legacy_reset_json);
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

        selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
        selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

        // Highlight only selected sequence buttton
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            // params[SEQUENCER_SELECTION_BUTTONS + i].setValue(selected_sequencer_index == i);
            lights[SEQUENCER_LIGHTS + i].setBrightness(selected_sequencer_index == i);
        }

        //
        // Set all of the sequence lengths by checking the corresponding knobs
        //
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            voltage_sequencers[i].setWindowEnd((int)params[SEQUENCER_LENGTH_KNOBS + i].getValue() - 1);
            gate_sequencers[i].setWindowEnd((int)params[SEQUENCER_LENGTH_KNOBS + i].getValue() - 1);
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

                float output = this->getScaledOutput(i);

                if (voltage_sequencers[i].sample_and_hold)
                {
                    if (gate_sequencers[i].getValue())
                        outputs[voltage_outputs[i]].setVoltage(output);
                }
                else
                {
                    outputs[voltage_outputs[i]].setVoltage(output);
                }
            }
        } // END IF NOT FROZEN

        else // IF FROZEN
        {
            // output values
            for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
            {
                float output = this->getScaledOutput(i);

                // Notice that this ignores sample + hold.  This is the main reason
                // for duplicating this code between the frozen/not frozen IF statments.
                outputs[voltage_outputs[i]].setVoltage(output);
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

     
    }
};
