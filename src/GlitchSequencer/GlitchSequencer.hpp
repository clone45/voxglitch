struct GlitchSequencer : VoxglitchModule
{
  CellularAutomatonSequencer sequencer;

  dsp::SchmittTrigger stepTrigger;
  dsp::SchmittTrigger resetTrigger;
  dsp::SchmittTrigger trigger_group_button_schmitt_trigger[8];
  dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_TRIGGER_GROUPS];

  unsigned int mode = PLAY_MODE;
  bool trigger_button_is_triggered[NUMBER_OF_TRIGGER_GROUPS];
  int selected_trigger_group_index = -1; // -1 means "none selected"
  long clock_ignore_on_reset = 0;

  bool trigger_results[NUMBER_OF_TRIGGER_GROUPS];

  enum ParamIds
  {
    LENGTH_KNOB,
    ENUMS(TRIGGER_GROUP_BUTTONS, NUMBER_OF_TRIGGER_GROUPS),
    NUM_PARAMS
  };

  enum InputIds {
    STEP_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };

  enum OutputIds
  {
    ENUMS(GATE_OUTPUTS, NUMBER_OF_TRIGGER_GROUPS),
    NUM_OUTPUTS
  };

  enum LightIds
  {
    ENUMS(TRIGGER_GROUP_LIGHTS, NUMBER_OF_TRIGGER_GROUPS),
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  GlitchSequencer()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(LENGTH_KNOB, 1, MAX_SEQUENCE_LENGTH, 16, "LengthKnob");

    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      configParam(TRIGGER_GROUP_BUTTONS + i, 0.f, 1.f, 0.f, "TriggerGroupButton");
    }
  }

  json_t *dataToJson() override
  {
    json_t *root = json_object();

    //
    // Prepare seed and trigger patterns for saving
    //

    std::string packed_seed_pattern = sequencer.packPattern(&sequencer.seed);

    std::string packed_trigger_patterns[NUMBER_OF_TRIGGER_GROUPS];
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

    // Process when the user presses one of the 8 buttons above the trigger outputs
    for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      trigger_button_is_triggered[i] = trigger_group_button_schmitt_trigger[i].process(params[TRIGGER_GROUP_BUTTONS + i].getValue());
      if(trigger_button_is_triggered[i]) toggleTriggerGroup(i);
    }

    // Highlight only selected sequence buttton
    for(int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      params[TRIGGER_GROUP_BUTTONS + i].setValue(selected_trigger_group_index == i);
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
    for(int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
      outputs[GATE_OUTPUTS + i].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));

      lights[TRIGGER_GROUP_LIGHTS + i].setBrightness(selected_trigger_group_index == i);
    }

    if (clock_ignore_on_reset > 0) clock_ignore_on_reset--;
  }
};
