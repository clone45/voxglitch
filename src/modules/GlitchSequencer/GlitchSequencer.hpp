struct GlitchSequencer : VoxglitchModule
{
  // The playback engine.  Its seed/triggers arrays always hold the ACTIVE
  // memory slot's pattern (edits from the screen write straight into them);
  // storeActivePattern()/loadActivePattern() sync them with memory[].
  CellularAutomatonSequencer sequencer;

  PatternMemory memory[NUMBER_OF_MEMORY_SLOTS];
  unsigned int memory_slot_index = 0;

  PatternMemory memory_clipboard;
  bool memory_clipboard_filled = false;

  dsp::SchmittTrigger stepTrigger;
  dsp::SchmittTrigger resetTrigger;
  dsp::PulseGenerator gateOutputPulseGenerators[NUMBER_OF_TRIGGER_GROUPS];

  // Which layer the screen is editing: EDIT_LAYER_WATCH, EDIT_LAYER_SEED,
  // or 1..8 for a trigger group.  Defaults to the seed so a fresh module
  // invites painting.
  int edit_layer = EDIT_LAYER_SEED;

  // Per-group flash levels for the screen tabs (set to 1 when a gate fires,
  // decayed in process(), read by the display widget).
  float tab_flash[NUMBER_OF_TRIGGER_GROUPS] = {};

  long clock_ignore_on_reset = 0;
  bool trigger_results[NUMBER_OF_TRIGGER_GROUPS] = {};

  enum ParamIds
  {
    LENGTH_KNOB,
    NUM_PARAMS
  };

  enum InputIds {
    STEP_INPUT,
    RESET_INPUT,
    MEM_INPUT,
    NUM_INPUTS
  };

  enum OutputIds
  {
    ENUMS(GATE_OUTPUTS, NUMBER_OF_TRIGGER_GROUPS),
    NUM_OUTPUTS
  };

  enum LightIds
  {
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  GlitchSequencer()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(LENGTH_KNOB, 1, MAX_SEQUENCE_LENGTH, 16, "Sequence Length");
    paramQuantities[LENGTH_KNOB]->snapEnabled = true;

    configInput(STEP_INPUT, "Step");
    configInput(RESET_INPUT, "Reset");
    configInput(MEM_INPUT, "Memory CV Select (0-10v)");

    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      configOutput(GATE_OUTPUTS + i, "Gate " + std::to_string(i + 1));
    }

    // Capture the sequencer's default demo pattern into slot 0 so a freshly
    // placed module behaves exactly like the pre-memory version.
    storeActivePattern();
  }

  // ── Memory slots ─────────────────────────────────────────────────────────

  void storeActivePattern()
  {
    PatternMemory *slot = &memory[memory_slot_index];
    slot->length = (unsigned int) params[LENGTH_KNOB].getValue();
    sequencer.copyPattern(&slot->seed, &sequencer.seed);
    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      sequencer.copyPattern(&slot->triggers[i], &sequencer.triggers[i]);
    }
  }

  void loadActivePattern()
  {
    PatternMemory *slot = &memory[memory_slot_index];
    // Move the LEN knob to this memory's stored length
    params[LENGTH_KNOB].setValue(slot->length);
    sequencer.setLength(slot->length);
    sequencer.copyPattern(&sequencer.seed, &slot->seed);
    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      sequencer.copyPattern(&sequencer.triggers[i], &slot->triggers[i]);
    }
  }

  bool memCableIsConnected()
  {
    return(inputs[MEM_INPUT].isConnected());
  }

  // Selecting a memory restarts the automaton from that memory's seed:
  // a memory is a fresh evolution, never a mid-flight pattern swap.
  void switchMemory(unsigned int new_slot)
  {
    if(new_slot >= NUMBER_OF_MEMORY_SLOTS) return;
    if(new_slot == memory_slot_index) return;

    storeActivePattern();
    memory_slot_index = new_slot;
    loadActivePattern();
    sequencer.restart_sequence();
  }

  // Copy / Paste / Clear, reachable by right-clicking a memory button on the
  // screen.  These work even while the MEM CV is patched: the CV owns which
  // memory PLAYS, these edit what a memory CONTAINS.
  void copyMemory(unsigned int slot_index)
  {
    if(slot_index >= NUMBER_OF_MEMORY_SLOTS) return;
    if(slot_index == memory_slot_index) storeActivePattern();
    memory_clipboard.copyFrom(&memory[slot_index]);
    memory_clipboard_filled = true;
  }

  void pasteMemory(unsigned int slot_index)
  {
    if(slot_index >= NUMBER_OF_MEMORY_SLOTS) return;
    if(! memory_clipboard_filled) return;
    memory[slot_index].copyFrom(&memory_clipboard);

    if(slot_index == memory_slot_index)
    {
      loadActivePattern();
      sequencer.restart_sequence();
    }
  }

  void clearMemory(unsigned int slot_index)
  {
    if(slot_index >= NUMBER_OF_MEMORY_SLOTS) return;
    memory[slot_index].clear();

    if(slot_index == memory_slot_index)
    {
      loadActivePattern();
      sequencer.restart_sequence();
    }
  }

  void toggleEditLayer(int layer)
  {
    edit_layer = (edit_layer == layer) ? EDIT_LAYER_WATCH : layer;
  }

  // ── Save / load ──────────────────────────────────────────────────────────

  json_t *dataToJson() override
  {
    storeActivePattern();

    json_t *root = json_object();

    //
    // Legacy keys: slot 0 is saved in the pre-memory format so a patch saved
    // here still opens (as its first memory) in older builds.
    //

    json_object_set_new(root, "seed_pattern", json_string(sequencer.packPattern(&memory[0].seed).c_str()));

    json_t *trigger_groups_json_array = json_array();
    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      json_array_append_new(trigger_groups_json_array, json_string(sequencer.packPattern(&memory[0].triggers[i]).c_str()));
    }
    json_object_set_new(root, "trigger_group_patterns", trigger_groups_json_array);

    //
    // All 16 memory slots
    //

    json_t *memory_slots_json_array = json_array();
    for(unsigned int s=0; s<NUMBER_OF_MEMORY_SLOTS; s++)
    {
      json_t *slot_json = json_object();
      json_object_set_new(slot_json, "length", json_integer(memory[s].length));
      json_object_set_new(slot_json, "seed", json_string(sequencer.packPattern(&memory[s].seed).c_str()));

      json_t *slot_triggers_json_array = json_array();
      for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
      {
        json_array_append_new(slot_triggers_json_array, json_string(sequencer.packPattern(&memory[s].triggers[i]).c_str()));
      }
      json_object_set_new(slot_json, "triggers", slot_triggers_json_array);

      json_array_append_new(memory_slots_json_array, slot_json);
    }
    json_object_set_new(root, "memory_slots", memory_slots_json_array);

    json_object_set_new(root, "selected_memory_index", json_integer(memory_slot_index));

    return root;
  }

  void dataFromJson(json_t *root) override
  {
    //
    // Legacy single-pattern patches load into slot 0
    //

    // Legacy patches carry the length only in the knob param, which Rack has
    // already restored by the time dataFromJson runs.
    memory[0].length = (unsigned int) params[LENGTH_KNOB].getValue();

    json_t *loaded_seed_pattern_json = json_object_get(root, "seed_pattern");
    if(loaded_seed_pattern_json) sequencer.unpackPattern(json_string_value(loaded_seed_pattern_json), &memory[0].seed);

    json_t *trigger_group_json_array = json_object_get(root, "trigger_group_patterns");
    if(trigger_group_json_array)
    {
      size_t i;
      json_t *loaded_trigger_pattern_json;

      json_array_foreach(trigger_group_json_array, i, loaded_trigger_pattern_json)
      {
        if(i < NUMBER_OF_TRIGGER_GROUPS) sequencer.unpackPattern(json_string_value(loaded_trigger_pattern_json), &memory[0].triggers[i]);
      }
    }

    //
    // Memory-aware patches override with all 16 slots
    //

    json_t *memory_slots_json_array = json_object_get(root, "memory_slots");
    if(memory_slots_json_array)
    {
      size_t s;
      json_t *slot_json;

      json_array_foreach(memory_slots_json_array, s, slot_json)
      {
        if(s >= NUMBER_OF_MEMORY_SLOTS) break;

        json_t *slot_length_json = json_object_get(slot_json, "length");
        if(slot_length_json)
        {
          unsigned int slot_length = json_integer_value(slot_length_json);
          if(slot_length >= 1 && slot_length <= MAX_SEQUENCE_LENGTH) memory[s].length = slot_length;
        }

        json_t *slot_seed_json = json_object_get(slot_json, "seed");
        if(slot_seed_json) sequencer.unpackPattern(json_string_value(slot_seed_json), &memory[s].seed);

        json_t *slot_triggers_json_array = json_object_get(slot_json, "triggers");
        if(slot_triggers_json_array)
        {
          size_t i;
          json_t *slot_trigger_json;
          json_array_foreach(slot_triggers_json_array, i, slot_trigger_json)
          {
            if(i < NUMBER_OF_TRIGGER_GROUPS) sequencer.unpackPattern(json_string_value(slot_trigger_json), &memory[s].triggers[i]);
          }
        }
      }
    }

    json_t *selected_memory_index_json = json_object_get(root, "selected_memory_index");
    if(selected_memory_index_json)
    {
      unsigned int index = json_integer_value(selected_memory_index_json);
      if(index < NUMBER_OF_MEMORY_SLOTS) memory_slot_index = index;
    }

    // It's necessary to restart the sequence because it copies the seed
    // into the current state.  Otherwise, the old default seed would still
    // be used for the first loop of the sequence.
    loadActivePattern();
    sequencer.restart_sequence();
  }

  // ── DSP ──────────────────────────────────────────────────────────────────

  void process(const ProcessArgs &args) override
  {
    bool trigger_output_pulse = false;

    // Set sequencer length based on LEN knob
    sequencer.setLength(params[LENGTH_KNOB].getValue());

    // Memory CV overrides the on-screen buttons while its cable is connected
    // (GrooveBox convention): 0-10v spread across memories 1-16.
    if(inputs[MEM_INPUT].isConnected())
    {
      int selection = (int) ((inputs[MEM_INPUT].getVoltage() / 10.0f) * NUMBER_OF_MEMORY_SLOTS);
      if(selection < 0) selection = 0;
      if(selection > NUMBER_OF_MEMORY_SLOTS - 1) selection = NUMBER_OF_MEMORY_SLOTS - 1;
      if(selection != (int) memory_slot_index) switchMemory(selection);
    }

    // Process Reset input
    if(resetTrigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
    {
      // Set up a (reverse) counter so that the clock input will ignore
      // incoming clock pulses for 1 millisecond after a reset input. This
      // is to comply with VCV Rack's standards.  See section "Timing" at
      // https://vcvrack.com/manual/VoltageStandards

      clock_ignore_on_reset = (long) (args.sampleRate / 100);
      stepTrigger.reset();
      sequencer.reset();
    }

    // Process Step Input
    if((clock_ignore_on_reset == 0) && stepTrigger.process(inputs[STEP_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
    {
      sequencer.step(trigger_results);

      for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
      {
        if(trigger_results[i])
        {
          gateOutputPulseGenerators[i].trigger(0.01f);
          tab_flash[i] = 1.0f;
        }
      }
    }

    // Output gates
    for(int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
      outputs[GATE_OUTPUTS + i].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
    }

    // Decay the tab flash levels (roughly 200ms from full to off)
    for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      tab_flash[i] -= args.sampleTime * 5.0f;
      if(tab_flash[i] < 0.0f) tab_flash[i] = 0.0f;
    }

    if (clock_ignore_on_reset > 0) clock_ignore_on_reset--;
  }
};
