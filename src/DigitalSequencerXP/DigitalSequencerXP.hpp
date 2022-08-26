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

  dsxp::VoltageSequencer voltage_sequencers[NUMBER_OF_SEQUENCERS];
  dsxp::VoltageSequencer *selected_voltage_sequencer;
  dsxp::GateSequencer gate_sequencers[NUMBER_OF_SEQUENCERS];
  dsxp::GateSequencer *selected_gate_sequencer;

  unsigned int selected_sequencer_index = 0;
  int voltage_outputs[NUMBER_OF_SEQUENCERS];
  int gate_outputs[NUMBER_OF_SEQUENCERS];
  int sequencer_step_inputs[NUMBER_OF_SEQUENCERS];
  bool step[NUMBER_OF_SEQUENCERS];

  dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_SEQUENCERS];
  double sample_rate;

  // There must be a better way...
  std::string labels[NUMBER_OF_SEQUENCERS] = {"","","","","","","","","","","","","","","",""};

  bool sequencer_button_is_triggered[NUMBER_OF_SEQUENCERS];

  std::string voltage_range_names[NUMBER_OF_VOLTAGE_RANGES] = {
    "0.0 to 10.0",
    "-10.0 to 10.0",
    "0.0 to 5.0",
    "-5.0 to 5.0",
    "0.0 to 3.0",
    "-3.0 to 3.0",
    "0.0 to 1.0",
    "-1.0 to 1.0"
  };

  std::string snap_division_names[NUMBER_OF_SNAP_DIVISIONS] = { "None", "8", "10", "12", "16", "24", "32", "36" };

  enum ParamIds {
    ENUMS(SEQUENCER_BUTTONS, NUMBER_OF_SEQUENCERS),
    FREEZE_TOGGLE,
    NUM_PARAMS
  };
  enum InputIds {
    POLY_STEP_INPUT,
    POLY_LENGTH_INPUT,
    RESET_INPUT,
    POLY_MOD_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    POLY_CV_OUTPUT,
    POLY_GATE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    ENUMS(SEQUENCER_LIGHTS, NUMBER_OF_SEQUENCERS),
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  DigitalSequencerXP()
  {

    selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
    selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // On boot, I seem to be getting some weird gate signals.  This keeps those
    // from triggering an output pulse when the module first loads.
    // Commented out while debugging.  It's safe to use this.
    clock_ignore_on_reset = (long) (44100 / 100);

  }

  void onRandomize() override {
    for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
    {
      for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
      {
        this->voltage_sequencers[sequencer_number].randomize();
        this->gate_sequencers[sequencer_number].randomize();
      }
    }
	}

  void copy(unsigned int src_sequencer_index, unsigned int dst_sequencer_index)
  {
    for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
    {
      this->voltage_sequencers[dst_sequencer_index].setValue(i,this->voltage_sequencers[src_sequencer_index].getValue(i));
      this->gate_sequencers[dst_sequencer_index].setValue(i,this->gate_sequencers[src_sequencer_index].getValue(i));
    }
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

    //
    // Save patterns
    //

    json_t *sequences_json_array = json_array();

    for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
    {
      json_t *pattern_json_array = json_array();

      for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
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

    for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
    {
      json_t *pattern_json_array = json_array();

      for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
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
    for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
    {
      json_array_append_new(sequencer_lengths_json_array, json_integer(this->voltage_sequencers[sequencer_number].getLength()));
    }
    json_object_set(json_root, "lengths", sequencer_lengths_json_array);
    json_decref(sequencer_lengths_json_array);

    //
    // Save sequencer voltage range index selections
    //
    json_t *sequencer_voltage_range_json_array = json_array();
    for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
    {
      json_array_append_new(sequencer_voltage_range_json_array, json_integer(this->voltage_sequencers[sequencer_number].voltage_range_index));
    }
    json_object_set(json_root, "voltage_ranges", sequencer_voltage_range_json_array);
    json_decref(sequencer_voltage_range_json_array);

    //
    // Save sequencer snap index selections
    //
    json_t *sequencer_snap_json_array = json_array();
    for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
    {
      json_array_append_new(sequencer_snap_json_array, json_integer(this->voltage_sequencers[sequencer_number].snap_division_index));
    }
    json_object_set(json_root, "snap_divisions", sequencer_snap_json_array);
    json_decref(sequencer_snap_json_array);

    //
    // Save sequencer sample and hold selections
    //
    json_t *sequencer_sh_json_array = json_array();
    for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
    {
      json_array_append_new(sequencer_sh_json_array, json_integer(this->voltage_sequencers[sequencer_number].sample_and_hold));
    }
    json_object_set(json_root, "sample_and_hold", sequencer_sh_json_array);
    json_decref(sequencer_sh_json_array);

    // Save Legacy Reset mode
    json_object_set_new(json_root, "legacy_reset", json_integer(legacy_reset));

    //
    // Save the labels
    //
    json_t *labels_json_array = json_array();
    for(int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
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

    //
    // Load patterns
    //

    json_t *pattern_arrays_data = json_object_get(json_root, "patterns");

    if(pattern_arrays_data)
    {
      size_t pattern_number;
      json_t *json_pattern_array;

      json_array_foreach(pattern_arrays_data, pattern_number, json_pattern_array)
      {
        for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
        {
          this->voltage_sequencers[pattern_number].setValue(i, json_real_value(json_array_get(json_pattern_array, i)));
        }
      }
    }

    //
    // Load gates
    //

    json_t *gates_arrays_data = json_object_get(json_root, "gates");

    if(gates_arrays_data)
    {
      size_t pattern_number;
      json_t *json_pattern_array;

      json_array_foreach(gates_arrays_data, pattern_number, json_pattern_array)
      {
        for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
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

    if(lengths_json_array)
    {
      size_t sequencer_number;
      json_t *length_json;

      json_array_foreach(lengths_json_array, sequencer_number, length_json)
      {
        this->voltage_sequencers[sequencer_number].setLength(json_integer_value(length_json));
        this->gate_sequencers[sequencer_number].setLength(json_integer_value(length_json));
      }
    }

    //
    // Load voltage ranges
    //
    json_t *voltage_ranges_json_array = json_object_get(json_root, "voltage_ranges");

    if(voltage_ranges_json_array)
    {
      size_t sequencer_number;
      json_t *voltage_range_json;

      json_array_foreach(voltage_ranges_json_array, sequencer_number, voltage_range_json)
      {
        this->voltage_sequencers[sequencer_number].voltage_range_index = json_integer_value(voltage_range_json);
      }
    }

    //
    // Load snap divisions
    //
    json_t *snap_divions_json_array = json_object_get(json_root, "snap_divisions");

    if(snap_divions_json_array)
    {
      size_t sequencer_number;
      json_t *snap_divion_json;

      json_array_foreach(snap_divions_json_array, sequencer_number, snap_divion_json)
      {
        this->voltage_sequencers[sequencer_number].snap_division_index = json_integer_value(snap_divion_json);
      }
    }

    //
    // Load Sample and Hold settings
    //
    json_t *sh_json_array = json_object_get(json_root, "sample_and_hold");

    if(sh_json_array)
    {
      size_t sequencer_number;
      json_t *sh_json;

      json_array_foreach(sh_json_array, sequencer_number, sh_json)
      {
        this->voltage_sequencers[sequencer_number].sample_and_hold = json_integer_value(sh_json);
      }
    }

    //
    // Load legacy reset flag
    //
    json_t* legacy_reset_json = json_object_get(json_root, "legacy_reset");
    if (legacy_reset_json) legacy_reset = json_integer_value(legacy_reset_json);

    //
    // Load the labels
    //
    json_t *labels_json = json_object_get(json_root, "labels");

    if(labels_json)
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

    for(unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      if(sequencer_button_triggers[i].process(params[SEQUENCER_BUTTONS + i].getValue()))
      {
        selected_sequencer_index = i;
      }
    }

    // Highlight only selected sequence buttton
    for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
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
      if(resetTrigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
      {
        // Set up a (reverse) counter so that the clock input will ignore
        // incoming clock pulses for 1 millisecond after a reset input. This
        // is to comply with VCV Rack's standards.  See section "Timing" at
        // https://vcvrack.com/manual/VoltageStandards

        clock_ignore_on_reset = (long) (args.sampleRate / 100);
        first_step = true;

        for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
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

      if(legacy_reset || clock_ignore_on_reset == 0)
      {
        // reset_first_step ensures that the first step of the sequence is not skipped
        // after a reset.  This functionality is disbled in legacy reset mode.
        bool reset_first_step = false;

        for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
        {
          if(step[i]) // Step == true means, "step the sequencer forward one step"
          {
            // If legacy reset is true or it's not the first step of the sequence
            // then go ahead and step the sequences.  Otherwise skip the first
            // step.
            if(legacy_reset || first_step == false)
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
            if(gate_sequencers[i].getValue()) gateOutputPulseGenerators[i].trigger(0.01f);
          }
        }

        // If the first step of the sequence was skipped, don't skip it again later on!
        if(reset_first_step == true) first_step = false;
      }


      // output values
      for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
      {
        if(voltage_sequencers[i].sample_and_hold)
        {
          if(gate_sequencers[i].getValue()) outputs[POLY_CV_OUTPUT].setVoltage(voltage_sequencers[i].getOutput(), i);
        }
        else
        {
          outputs[POLY_CV_OUTPUT].setVoltage(voltage_sequencers[i].getOutput(), i);
        }
      }
      outputs[POLY_CV_OUTPUT].setChannels(NUMBER_OF_SEQUENCERS);

    } // END IF NOT FROZEN

    else // IF FROZEN
    {
      // output values
      for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
      {
        // Notice that this ignores sample + hold.  This is the main reason
        // for duplicating this code between the frozen/not frozen IF statments.
        outputs[POLY_CV_OUTPUT].setVoltage(voltage_sequencers[i].getOutput(), i);
      }

      if(frozen_trigger_gate)
      {
        gateOutputPulseGenerators[selected_sequencer_index].trigger(0.01f);
        frozen_trigger_gate = false;
      }
    }

    //
    // process trigger outputs
    //
    for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
      outputs[POLY_GATE_OUTPUT].setVoltage((trigger_output_pulse ? 10.0f : 0.0f), i);
    }
    outputs[POLY_GATE_OUTPUT].setChannels(NUMBER_OF_SEQUENCERS);

    //
    // Adjust timers
    //
    if (clock_ignore_on_reset > 0) clock_ignore_on_reset--;
    if (tooltip_timer > 0) tooltip_timer--;

    // Light up currently selected sequencer lamp
    for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      if(i == selected_sequencer_index)
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
    if(number_of_step_input_channels > 1)
    {
      for(unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
      {
        step[i] = stepTriggers[i].process(rescale(inputs[POLY_STEP_INPUT].getVoltage(i), 0.0f, 10.0f, 0.f, 1.f));
      }
    }
    // If a monophonic (or no) input is being used
    else
    {
      // Poll the first step input channel
      bool stepped = stepTriggers[0].process(rescale(inputs[POLY_STEP_INPUT].getVoltage(0), 0.0f, 10.0f, 0.f, 1.f));

      // Apply the value found in the previous step to channels 2, 3, 4, etc...
      for(unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++) step[i] = stepped;
    }
  }

  void set_sequencer_lengths()
  {
    for(unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      if(i < (unsigned int) inputs[POLY_LENGTH_INPUT].getChannels()) // If there's an input controlling this sequencer
      {
        float length_input = inputs[POLY_LENGTH_INPUT].getVoltage(i);
        int length = ((length_input / 10.0) * 32) + 1;
        length = clamp(length, 1, 32);

        voltage_sequencers[i].setLength((int) length);
        gate_sequencers[i].setLength((int) length);
      }
    }
  }

};
