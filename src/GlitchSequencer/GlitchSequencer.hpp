struct GlitchSequencer : Module
{
  CellularAutomatonSequencer sequencer;

  dsp::SchmittTrigger stepTrigger;
  dsp::SchmittTrigger resetTrigger;
  dsp::SchmittTrigger trigger_group_button_schmitt_trigger[5];
  dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_TRIGGER_GROUPS];

  unsigned int mode = PLAY_MODE;
  bool trigger_button_is_triggered[NUMBER_OF_TRIGGER_GROUPS];
  unsigned int trigger_group_buttons[NUMBER_OF_TRIGGER_GROUPS];
  unsigned int gate_outputs[NUMBER_OF_TRIGGER_GROUPS];
  int selected_trigger_group_index = -1; // -1 means "none selected"
  long clock_ignore_on_reset = 0;

  bool trigger_results[NUMBER_OF_TRIGGER_GROUPS];

  enum ParamIds {
    LENGTH_KNOB,
    TRIGGER_GROUP_1_BUTTON,
    TRIGGER_GROUP_2_BUTTON,
    TRIGGER_GROUP_3_BUTTON,
    TRIGGER_GROUP_4_BUTTON,
    TRIGGER_GROUP_5_BUTTON,
    NUM_PARAMS
  };
  enum InputIds {
    STEP_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    GATE_OUTPUT_1,
    GATE_OUTPUT_2,
    GATE_OUTPUT_3,
    GATE_OUTPUT_4,
    GATE_OUTPUT_5,
    NUM_OUTPUTS
  };
  enum LightIds {
    TRIGGER_GROUP_1_LIGHT,
    TRIGGER_GROUP_2_LIGHT,
    TRIGGER_GROUP_3_LIGHT,
    TRIGGER_GROUP_4_LIGHT,
    TRIGGER_GROUP_5_LIGHT,
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  GlitchSequencer()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(LENGTH_KNOB, 1, MAX_SEQUENCE_LENGTH, 16, "LengthKnob");
    configParam(TRIGGER_GROUP_1_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup1Button");
    configParam(TRIGGER_GROUP_2_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup2Button");
    configParam(TRIGGER_GROUP_3_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup3Button");
    configParam(TRIGGER_GROUP_4_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup4Button");
    configParam(TRIGGER_GROUP_5_BUTTON, 0.f, 1.f, 0.f, "TriggerGroup5Button");

    trigger_group_buttons[0] = TRIGGER_GROUP_1_BUTTON;
    trigger_group_buttons[1] = TRIGGER_GROUP_2_BUTTON;
    trigger_group_buttons[2] = TRIGGER_GROUP_3_BUTTON;
    trigger_group_buttons[3] = TRIGGER_GROUP_4_BUTTON;
    trigger_group_buttons[4] = TRIGGER_GROUP_5_BUTTON;

    gate_outputs[0] = GATE_OUTPUT_1;
    gate_outputs[1] = GATE_OUTPUT_2;
    gate_outputs[2] = GATE_OUTPUT_3;
    gate_outputs[3] = GATE_OUTPUT_4;
    gate_outputs[4] = GATE_OUTPUT_5;
  }

  json_t *dataToJson() override
  {
    json_t *root = json_object();

    //
    // Prepare seed and trigger patterns for saving
    //

    std::string packed_seed_pattern = sequencer.packPattern(&sequencer.seed);

    std::string packed_trigger_patterns[5];
    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      packed_trigger_patterns[i] = sequencer.packPattern(&sequencer.triggers[i]);
    }

    //
    // Save seed pattern
    //
    json_object_set_new(root, "seed_pattern", json_string(packed_seed_pattern.c_str()));


    //
    // Save trigger patterns
    //

    json_t *trigger_groups_json_array = json_array();

    for(int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      json_array_append_new(trigger_groups_json_array, json_string(packed_trigger_patterns[i].c_str()));
    }

    json_object_set(root, "trigger_group_patterns", trigger_groups_json_array);
    json_decref(trigger_groups_json_array);

    // Done
    return root;
  }

  void dataFromJson(json_t *root) override
  {
    // Load seed_pattern
    json_t *loaded_seed_pattern_json = json_object_get(root, ("seed_pattern"));
    if(loaded_seed_pattern_json) sequencer.unpackPattern(json_string_value(loaded_seed_pattern_json), &sequencer.seed);

    // It's necessary to restart the sequence because it copies the seed
    // into the current state.  Otherwise, the old default seed would still
    // be used for the first loop of the sequence.
    sequencer.restart_sequence();

    //
    // load trigger group patterns
    //

    json_t *trigger_group_json_array = json_object_get(root, "trigger_group_patterns");

    if(trigger_group_json_array)
    {
      size_t i;
      json_t *loaded_trigger_pattern_json;

      json_array_foreach(trigger_group_json_array, i, loaded_trigger_pattern_json)
      {
        sequencer.unpackPattern(json_string_value(loaded_trigger_pattern_json), &sequencer.triggers[i]);
      }
    }
  }

  void toggleTriggerGroup(int index)
  {
    if(selected_trigger_group_index == index)
    {
      selected_trigger_group_index = -1;
      mode = PLAY_MODE;
    }
    else
    {
      selected_trigger_group_index = index;
      mode = EDIT_TRIGGERS_MODE;
    }
  }

  void process(const ProcessArgs &args) override
  {
    bool trigger_output_pulse = false;

    // Set sequencer length based on LEN knob
    sequencer.setLength(params[LENGTH_KNOB].getValue());

    // Process Reset input
    if(resetTrigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      // Set up a (reverse) counter so that the clock input will ignore
      // incoming clock pulses for 1 millisecond after a reset input. This
      // is to comply with VCV Rack's standards.  See section "Timing" at
      // https://vcvrack.com/manual/VoltageStandards

      clock_ignore_on_reset = (long) (args.sampleRate / 100);
      stepTrigger.reset();
      sequencer.reset();
    }

    // Process when the user presses one of the 5 buttons above the trigger outputs
    for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      trigger_button_is_triggered[i] = trigger_group_button_schmitt_trigger[i].process(params[trigger_group_buttons[i]].getValue());
      if(trigger_button_is_triggered[i]) toggleTriggerGroup(i);
    }

    // Process Step Input
    if((clock_ignore_on_reset == 0) && stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      sequencer.step(trigger_results);

      for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
      {
        if(trigger_results[i]) gateOutputPulseGenerators[i].trigger(0.01f);
      }
    }

    // Output gates
    for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
      outputs[gate_outputs[i]].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
    }

    // Trigger group selection lights
    lights[TRIGGER_GROUP_1_LIGHT].setBrightness(selected_trigger_group_index == 0);
    lights[TRIGGER_GROUP_2_LIGHT].setBrightness(selected_trigger_group_index == 1);
    lights[TRIGGER_GROUP_3_LIGHT].setBrightness(selected_trigger_group_index == 2);
    lights[TRIGGER_GROUP_4_LIGHT].setBrightness(selected_trigger_group_index == 3);
    lights[TRIGGER_GROUP_5_LIGHT].setBrightness(selected_trigger_group_index == 4);

    if (clock_ignore_on_reset > 0) clock_ignore_on_reset--;
  }
};
