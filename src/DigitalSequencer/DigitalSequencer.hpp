struct DigitalSequencer : Module
{
  dsp::SchmittTrigger stepTrigger;
  dsp::SchmittTrigger sequencer_step_triggers[NUMBER_OF_SEQUENCERS];
  dsp::SchmittTrigger resetTrigger;

  dsp::SchmittTrigger sequencer_1_button_trigger;
  dsp::SchmittTrigger sequencer_2_button_trigger;
  dsp::SchmittTrigger sequencer_3_button_trigger;
  dsp::SchmittTrigger sequencer_4_button_trigger;
  dsp::SchmittTrigger sequencer_5_button_trigger;
  dsp::SchmittTrigger sequencer_6_button_trigger;

  long clock_ignore_on_reset = 0;
  unsigned int tooltip_timer = 0;

  VoltageSequencer voltage_sequencers[NUMBER_OF_SEQUENCERS];
  VoltageSequencer *selected_voltage_sequencer;

  GateSequencer gate_sequencers[NUMBER_OF_SEQUENCERS];
  GateSequencer *selected_gate_sequencer;

  int selected_sequencer_index = 0;
  // int previously_selected_sequencer_index = -1;
  int voltage_outputs[NUMBER_OF_SEQUENCERS];
  int gate_outputs[NUMBER_OF_SEQUENCERS];
  int sequencer_step_inputs[NUMBER_OF_SEQUENCERS];

  dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_SEQUENCERS];
  double sample_rate;

  bool sequencer_1_button_is_triggered;
  bool sequencer_2_button_is_triggered;
  bool sequencer_3_button_is_triggered;
  bool sequencer_4_button_is_triggered;
  bool sequencer_5_button_is_triggered;
  bool sequencer_6_button_is_triggered;

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
    SEQUENCE_SELECTION_KNOB,
    SEQUENCER_1_LENGTH_KNOB,
    SEQUENCER_2_LENGTH_KNOB,
    SEQUENCER_3_LENGTH_KNOB,
    SEQUENCER_4_LENGTH_KNOB,
    SEQUENCER_5_LENGTH_KNOB,
    SEQUENCER_6_LENGTH_KNOB,

    SEQUENCE_START_KNOB,
    SEQUENCER_1_BUTTON,
    SEQUENCER_2_BUTTON,
    SEQUENCER_3_BUTTON,
    SEQUENCER_4_BUTTON,
    SEQUENCER_5_BUTTON,
    SEQUENCER_6_BUTTON,
    NUM_PARAMS
  };
  enum InputIds {
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
  enum OutputIds {
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
  enum LightIds {
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
    configParam(SEQUENCER_1_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "SequenceLengthKnob");
    configParam(SEQUENCER_2_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer2LengthKnob");
    configParam(SEQUENCER_3_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer3LengthKnob");
    configParam(SEQUENCER_4_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer4LengthKnob");
    configParam(SEQUENCER_5_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer5LengthKnob");
    configParam(SEQUENCER_6_LENGTH_KNOB, 1, MAX_SEQUENCER_STEPS, MAX_SEQUENCER_STEPS, "Sequencer6LengthKnob");

    configParam(SEQUENCER_1_BUTTON, 0.f, 1.f, 0.f, "Sequence1Button");
    configParam(SEQUENCER_2_BUTTON, 0.f, 1.f, 0.f, "Sequence2Button");
    configParam(SEQUENCER_3_BUTTON, 0.f, 1.f, 0.f, "Sequence3Button");
    configParam(SEQUENCER_4_BUTTON, 0.f, 1.f, 0.f, "Sequence4Button");
    configParam(SEQUENCER_5_BUTTON, 0.f, 1.f, 0.f, "Sequence5Button");
    configParam(SEQUENCER_6_BUTTON, 0.f, 1.f, 0.f, "Sequence6Button");
  }

  /*
  ==================================================================================================================================================
  ___                 _
  / / |               | |
  ___  __ ___   _____  / /| | ___   __ _  __| |
  / __|/ _` \ \ / / _ \ / / | |/ _ \ / _` |/ _` |
  \__ \ (_| |\ V /  __// /  | | (_) | (_| | (_| |
  |___/\__,_| \_/ \___/_/   |_|\___/ \__,_|\__,_|

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
        json_array_append_new(pattern_json_array, json_integer(this->voltage_sequencers[sequencer_number].getValue(i)));
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
          this->voltage_sequencers[pattern_number].setValue(i, json_integer_value(json_array_get(json_pattern_array, i)));
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
    bool trigger_output_pulse = false;
    this->sample_rate = args.sampleRate;

    selected_voltage_sequencer = &voltage_sequencers[selected_sequencer_index];
    selected_gate_sequencer = &gate_sequencers[selected_sequencer_index];

    sequencer_1_button_is_triggered = sequencer_1_button_trigger.process(params[SEQUENCER_1_BUTTON].getValue());
    sequencer_2_button_is_triggered = sequencer_2_button_trigger.process(params[SEQUENCER_2_BUTTON].getValue());
    sequencer_3_button_is_triggered = sequencer_3_button_trigger.process(params[SEQUENCER_3_BUTTON].getValue());
    sequencer_4_button_is_triggered = sequencer_4_button_trigger.process(params[SEQUENCER_4_BUTTON].getValue());
    sequencer_5_button_is_triggered = sequencer_5_button_trigger.process(params[SEQUENCER_5_BUTTON].getValue());
    sequencer_6_button_is_triggered = sequencer_6_button_trigger.process(params[SEQUENCER_6_BUTTON].getValue());

    if(sequencer_1_button_is_triggered) selected_sequencer_index = 0;
    if(sequencer_2_button_is_triggered) selected_sequencer_index = 1;
    if(sequencer_3_button_is_triggered) selected_sequencer_index = 2;
    if(sequencer_4_button_is_triggered) selected_sequencer_index = 3;
    if(sequencer_5_button_is_triggered) selected_sequencer_index = 4;
    if(sequencer_6_button_is_triggered) selected_sequencer_index = 5;

    voltage_sequencers[0].setLength(clamp((int) params[SEQUENCER_1_LENGTH_KNOB].getValue(), 1, 32));
    voltage_sequencers[1].setLength(clamp((int) params[SEQUENCER_2_LENGTH_KNOB].getValue(), 1, 32));
    voltage_sequencers[2].setLength(clamp((int) params[SEQUENCER_3_LENGTH_KNOB].getValue(), 1, 32));
    voltage_sequencers[3].setLength(clamp((int) params[SEQUENCER_4_LENGTH_KNOB].getValue(), 1, 32));
    voltage_sequencers[4].setLength(clamp((int) params[SEQUENCER_5_LENGTH_KNOB].getValue(), 1, 32));
    voltage_sequencers[5].setLength(clamp((int) params[SEQUENCER_6_LENGTH_KNOB].getValue(), 1, 32));

    gate_sequencers[0].setLength(clamp((int) params[SEQUENCER_1_LENGTH_KNOB].getValue(), 1, 32));
    gate_sequencers[1].setLength(clamp((int) params[SEQUENCER_2_LENGTH_KNOB].getValue(), 1, 32));
    gate_sequencers[2].setLength(clamp((int) params[SEQUENCER_3_LENGTH_KNOB].getValue(), 1, 32));
    gate_sequencers[3].setLength(clamp((int) params[SEQUENCER_4_LENGTH_KNOB].getValue(), 1, 32));
    gate_sequencers[4].setLength(clamp((int) params[SEQUENCER_5_LENGTH_KNOB].getValue(), 1, 32));
    gate_sequencers[5].setLength(clamp((int) params[SEQUENCER_6_LENGTH_KNOB].getValue(), 1, 32));

    // On incoming RESET, reset the sequencers
    if(resetTrigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      // Set up a (reverse) counter so that the clock input will ignore
      // incoming clock pulses for 1 millisecond after a reset input. This
      // is to comply with VCV Rack's standards.  See section "Timing" at
      // https://vcvrack.com/manual/VoltageStandards

      clock_ignore_on_reset = (long) (args.sampleRate / 100);

      stepTrigger.reset();

      for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
      {
        sequencer_step_triggers[i].reset();
        voltage_sequencers[i].reset();
        gate_sequencers[i].reset();
      }
    }
    else if(clock_ignore_on_reset == 0)
    {
      // Step ALL of the sequencers
      bool global_step_trigger = stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f));
      bool step;

      for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
      {
        step = false;

        if(inputs[sequencer_step_inputs[i]].isConnected() == false)
        {
          if(global_step_trigger) step = true;
        }
        else if (sequencer_step_triggers[i].process(rescale(inputs[sequencer_step_inputs[i]].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
        {
          step = true;
        }

        if(step)
        {
          voltage_sequencers[i].step();
          gate_sequencers[i].step();
          if(gate_sequencers[i].getValue()) gateOutputPulseGenerators[i].trigger(0.01f);
        }
      }
    }

    // output values
    for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      if(voltage_sequencers[i].sample_and_hold)
      {
        if(gate_sequencers[i].getValue()) outputs[voltage_outputs[i]].setVoltage(voltage_sequencers[i].getOutput());
      }
      else
      {
        outputs[voltage_outputs[i]].setVoltage(voltage_sequencers[i].getOutput());
      }
    }

    // process trigger outputs
    for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
      outputs[gate_outputs[i]].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
    }

    if (clock_ignore_on_reset > 0) clock_ignore_on_reset--;
    if (tooltip_timer > 0) tooltip_timer--;

    lights[SEQUENCER_1_LIGHT].setBrightness(selected_sequencer_index == 0);
    lights[SEQUENCER_2_LIGHT].setBrightness(selected_sequencer_index == 1);
    lights[SEQUENCER_3_LIGHT].setBrightness(selected_sequencer_index == 2);
    lights[SEQUENCER_4_LIGHT].setBrightness(selected_sequencer_index == 3);
    lights[SEQUENCER_5_LIGHT].setBrightness(selected_sequencer_index == 4);
    lights[SEQUENCER_6_LIGHT].setBrightness(selected_sequencer_index == 5);
  }

};
