struct DigitalSequencerWidget : VoxglitchModuleWidget
{
  DigitalSequencer *module;
  int copy_sequencer_index = -1;

  DigitalSequencerWidget(DigitalSequencer *module)
  {
    this->module = module;
    setModule(module);

    // Load and apply theme
    theme.load("digital_sequencer");
    applyTheme();

 
    // =================== PLACE COMPONENTS ====================================

    if (theme.showScrews())
    {
      addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, 0)));
      addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
      addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
      addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    // Step
    addInput(createInputCentered<VoxglitchInputPort>(themePos("STEP_INPUT"), module, DigitalSequencer::STEP_INPUT));

    // Reset
    addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, DigitalSequencer::RESET_INPUT));

    // 6 step inputs
    addInput(createInputCentered<VoxglitchInputPort>(themePos("SEQUENCER_1_STEP_INPUT"), module, DigitalSequencer::SEQUENCER_1_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("SEQUENCER_2_STEP_INPUT"), module, DigitalSequencer::SEQUENCER_2_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("SEQUENCER_3_STEP_INPUT"), module, DigitalSequencer::SEQUENCER_3_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("SEQUENCER_4_STEP_INPUT"), module, DigitalSequencer::SEQUENCER_4_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("SEQUENCER_5_STEP_INPUT"), module, DigitalSequencer::SEQUENCER_5_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("SEQUENCER_6_STEP_INPUT"), module, DigitalSequencer::SEQUENCER_6_STEP_INPUT));

    // step length attenuators
    auto L1 = createParamCentered<VoxglitchAttenuator>(themePos("SEQUENCER_1_LENGTH_KNOB"), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 0);
    dynamic_cast<Knob *>(L1)->snap = true;
    addParam(L1);

    auto L2 = createParamCentered<VoxglitchAttenuator>(themePos("SEQUENCER_2_LENGTH_KNOB"), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 1);
    dynamic_cast<Knob *>(L2)->snap = true;
    addParam(L2);

    auto L3 = createParamCentered<VoxglitchAttenuator>(themePos("SEQUENCER_3_LENGTH_KNOB"), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 2);
    dynamic_cast<Knob *>(L3)->snap = true;
    addParam(L3);

    auto L4 = createParamCentered<VoxglitchAttenuator>(themePos("SEQUENCER_4_LENGTH_KNOB"), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 3);
    dynamic_cast<Knob *>(L4)->snap = true;
    addParam(L4);

    auto L5 = createParamCentered<VoxglitchAttenuator>(themePos("SEQUENCER_5_LENGTH_KNOB"), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 4);
    dynamic_cast<Knob *>(L5)->snap = true;
    addParam(L5);
    
    auto L6 = createParamCentered<VoxglitchAttenuator>(themePos("SEQUENCER_6_LENGTH_KNOB"), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 5);
    dynamic_cast<Knob *>(L6)->snap = true;
    addParam(L6);

    // Sequence selection buttons
    addParam(createParamCentered<VoxglitchRoundToggleLampSwitch>(themePos("SEQUENCER_1_BUTTON"), module, DigitalSequencer::SEQUENCER_SELECTION_BUTTONS + 0));
    addParam(createParamCentered<VoxglitchRoundToggleLampSwitch>(themePos("SEQUENCER_2_BUTTON"), module, DigitalSequencer::SEQUENCER_SELECTION_BUTTONS + 1));
    addParam(createParamCentered<VoxglitchRoundToggleLampSwitch>(themePos("SEQUENCER_3_BUTTON"), module, DigitalSequencer::SEQUENCER_SELECTION_BUTTONS + 2));
    addParam(createParamCentered<VoxglitchRoundToggleLampSwitch>(themePos("SEQUENCER_4_BUTTON"), module, DigitalSequencer::SEQUENCER_SELECTION_BUTTONS + 3));
    addParam(createParamCentered<VoxglitchRoundToggleLampSwitch>(themePos("SEQUENCER_5_BUTTON"), module, DigitalSequencer::SEQUENCER_SELECTION_BUTTONS + 4));
    addParam(createParamCentered<VoxglitchRoundToggleLampSwitch>(themePos("SEQUENCER_6_BUTTON"), module, DigitalSequencer::SEQUENCER_SELECTION_BUTTONS + 5));

    // CV outputs
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ1_CV_OUTPUT"), module, DigitalSequencer::SEQ1_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ2_CV_OUTPUT"), module, DigitalSequencer::SEQ2_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ3_CV_OUTPUT"), module, DigitalSequencer::SEQ3_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ4_CV_OUTPUT"), module, DigitalSequencer::SEQ4_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ5_CV_OUTPUT"), module, DigitalSequencer::SEQ5_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ6_CV_OUTPUT"), module, DigitalSequencer::SEQ6_CV_OUTPUT));

    // Gate outputs
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ1_GATE_OUTPUT"), module, DigitalSequencer::SEQ1_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ2_GATE_OUTPUT"), module, DigitalSequencer::SEQ2_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ3_GATE_OUTPUT"), module, DigitalSequencer::SEQ3_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ4_GATE_OUTPUT"), module, DigitalSequencer::SEQ4_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ5_GATE_OUTPUT"), module, DigitalSequencer::SEQ5_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SEQ6_GATE_OUTPUT"), module, DigitalSequencer::SEQ6_GATE_OUTPUT));

    // Main voltage sequencer display
    VoltageSequencerDisplay *voltage_sequencer_display = new VoltageSequencerDisplay();
    voltage_sequencer_display->box.pos = mm2px(themePos("CV_SEQUENCER"));
    voltage_sequencer_display->module = module;
    addChild(voltage_sequencer_display);

    GateSequencerDisplay *gates_display = new GateSequencerDisplay();
    gates_display->box.pos = mm2px(themePos("GATE_SEQUENCER"));
    gates_display->module = module;
    addChild(gates_display);

  }

  void addLayers()
  {
  }

  struct LengthDisplay : TransparentWidget
  {
    DigitalSequencer *module;
    std::shared_ptr<Font> font;
    unsigned int sequencer_number = 0;

    void draw(const DrawArgs &args) override
    {
      const auto vg = args.vg;

      nvgSave(vg);

      std::string text_to_display = "16";

      if (module)
      {
        text_to_display = std::to_string(module->voltage_sequencers[sequencer_number].getLength());
      }

      std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
      if (font)
      {
        nvgFontSize(vg, 9);
        nvgFontFaceId(vg, font->handle);
        nvgTextAlign(vg, NVG_ALIGN_CENTER);
        nvgTextLetterSpacing(vg, -1);
        nvgFillColor(vg, nvgRGBA(235, 229, 222, 240));
        nvgText(vg, 0, 0, text_to_display.c_str(), NULL);
      }

      nvgRestore(vg);
    }
  };

  //
  // ==========================================================================================
  // MENUS
  // ==========================================================================================
  //

  struct AllInputSnapsValueItem : MenuItem
  {
    DigitalSequencer *module;
    int snap_division_index = 0;

    void onAction(const event::Action &e) override
    {
      /* 

      TODO: I'll need to convert the snap division indexes to just snap divisions,
      then use module->voltage_sequencers[i].setSnapDivision(snap_division)
    
      for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
      {
        module->voltage_sequencers[i].snap_division_index = snap_division_index;
      }
      */
    }
  };

  /*
  struct AllInputSnapsItem : MenuItem
  {
    DigitalSequencer *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      for (unsigned int i = 0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
      {
        AllInputSnapsValueItem *all_input_snaps_value_item = createMenuItem<AllInputSnapsValueItem>(module->snap_division_names[i]);
        all_input_snaps_value_item->module = module;
        all_input_snaps_value_item->snap_division_index = i;
        menu->addChild(all_input_snaps_value_item);
      }

      return menu;
    }
  };

  struct AllOutputRangesValueItem : MenuItem
  {
    DigitalSequencer *module;
    int range_index = 0;

    void onAction(const event::Action &e) override
    {
      for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
      {
        module->voltage_sequencers[i].voltage_range_index = range_index;
      }
    }
  };

  struct AllOutputRangesItem : MenuItem
  {
    DigitalSequencer *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      for (unsigned int i = 0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
      {
        AllOutputRangesValueItem *all_output_ranges_value_item = createMenuItem<AllOutputRangesValueItem>(module->voltage_range_names[i]);
        all_output_ranges_value_item->module = module;
        all_output_ranges_value_item->range_index = i;
        menu->addChild(all_output_ranges_value_item);
      }

      return menu;
    }
  };

  struct AllSampleAndHoldsValueItem : MenuItem
  {
    DigitalSequencer *module;
    bool value = false;

    void onAction(const event::Action &e) override
    {
      for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
      {
        module->voltage_sequencers[i].sample_and_hold = value;
      }
    }
  };

  struct AllSampleAndHoldsItem : MenuItem
  {
    DigitalSequencer *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      AllSampleAndHoldsValueItem *sample_and_hold_off = createMenuItem<AllSampleAndHoldsValueItem>("Off");
      sample_and_hold_off->module = module;
      sample_and_hold_off->value = false;
      menu->addChild(sample_and_hold_off);

      AllSampleAndHoldsValueItem *sample_and_hold_on = createMenuItem<AllSampleAndHoldsValueItem>("On");
      sample_and_hold_on->module = module;
      sample_and_hold_on->value = true;
      menu->addChild(sample_and_hold_on);

      return menu;
    }
  };

  struct AllSequencersItem : MenuItem
  {
    DigitalSequencer *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      AllOutputRangesItem *all_output_ranges_item = createMenuItem<AllOutputRangesItem>("Output Range", RIGHT_ARROW);
      all_output_ranges_item->module = module;
      menu->addChild(all_output_ranges_item);

      AllInputSnapsItem *all_input_snaps_item = createMenuItem<AllInputSnapsItem>("Snap", RIGHT_ARROW);
      all_input_snaps_item->module = module;
      menu->addChild(all_input_snaps_item);

      AllSampleAndHoldsItem *all_sample_and_holds_items = createMenuItem<AllSampleAndHoldsItem>("Sample & Hold", RIGHT_ARROW);
      all_sample_and_holds_items->module = module;
      menu->addChild(all_sample_and_holds_items);

      return menu;
    }
  };

  struct InputSnapValueItem : MenuItem
  {
    DigitalSequencer *module;
    int snap_division_index = 0;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override
    {
      module->voltage_sequencers[sequencer_number].snap_division_index = snap_division_index;
    }
  };

  struct InputSnapItem : MenuItem
  {
    DigitalSequencer *module;
    int sequencer_number = 0;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      for (unsigned int i = 0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
      {
        InputSnapValueItem *input_snap_value_item = createMenuItem<InputSnapValueItem>(module->snap_division_names[i], CHECKMARK(module->voltage_sequencers[sequencer_number].snap_division_index == i));
        input_snap_value_item->module = module;
        input_snap_value_item->snap_division_index = i;
        input_snap_value_item->sequencer_number = this->sequencer_number;
        menu->addChild(input_snap_value_item);
      }

      return menu;
    }
  };

  struct OutputRangeValueItem : MenuItem
  {
    DigitalSequencer *module;
    int range_index = 0;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override
    {
      module->voltage_sequencers[sequencer_number].voltage_range_index = range_index;
    }
  };

  struct OutputRangeItem : MenuItem
  {
    DigitalSequencer *module;
    int sequencer_number = 0;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      for (unsigned int i = 0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
      {
        OutputRangeValueItem *output_range_value_menu_item = createMenuItem<OutputRangeValueItem>(module->voltage_range_names[i], CHECKMARK(module->voltage_sequencers[sequencer_number].voltage_range_index == i));
        output_range_value_menu_item->module = module;
        output_range_value_menu_item->range_index = i;
        output_range_value_menu_item->sequencer_number = this->sequencer_number;
        menu->addChild(output_range_value_menu_item);
      }

      return menu;
    }
  };

  */

  struct QuickKeyMenu : MenuItem
  {
    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      menu->addChild(createMenuLabel("      f : Toggle Freeze Mode (for easy editing)"));
      menu->addChild(createMenuLabel("      g : When frozen, press 'g' to send gate out"));
      menu->addChild(createMenuLabel(""));
      menu->addChild(createMenuLabel("      r : Randomize gate or voltage sequence"));
      menu->addChild(createMenuLabel("      ↑ : Nudge up voltage for hovered step"));
      menu->addChild(createMenuLabel("      ↓ : Nudge down voltage for hovered step"));
      menu->addChild(createMenuLabel("      → : Shift hovered sequence to the right"));
      menu->addChild(createMenuLabel("      ← : Shift hovered sequence to the left"));
      menu->addChild(createMenuLabel("    1-6 : Quickly select active sequencer"));
      menu->addChild(createMenuLabel("ctrl-c  : copy selected sequence"));
      menu->addChild(createMenuLabel("ctrl-v  : paste selected sequence"));

      return menu;
    }
  };

  struct ResetOnNextOption : MenuItem
  {
    DigitalSequencer *module;

    void onAction(const event::Action &e) override
    {
      module->legacy_reset = false;
    }
  };

  struct ResetInstantOption : MenuItem
  {
    DigitalSequencer *module;

    void onAction(const event::Action &e) override
    {
      module->legacy_reset = true;
    }
  };

  struct ResetModeItem : MenuItem
  {
    DigitalSequencer *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      ResetOnNextOption *reset_on_next = createMenuItem<ResetOnNextOption>("Next clock input.", CHECKMARK(!module->legacy_reset));
      reset_on_next->module = module;
      menu->addChild(reset_on_next);

      ResetInstantOption *reset_instant = createMenuItem<ResetInstantOption>("Instant", CHECKMARK(module->legacy_reset));
      reset_instant->module = module;
      menu->addChild(reset_instant);

      return menu;
    }
  };

  // Sample and Hold values
  struct SampleAndHoldItem : MenuItem
  {
    DigitalSequencer *module;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override
    {
      module->voltage_sequencers[sequencer_number].sample_and_hold ^= true; // flip the value
    }
  };

  struct SequencerItem : MenuItem
  {
    DigitalSequencer *module;
    unsigned int sequencer_number = 0;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      /* TODO: restore this
      OutputRangeItem *output_range_item = createMenuItem<OutputRangeItem>("Output Range", RIGHT_ARROW);
      output_range_item->sequencer_number = this->sequencer_number;
      output_range_item->module = module;
      menu->addChild(output_range_item);

      InputSnapItem *input_snap_item = createMenuItem<InputSnapItem>("Snap", RIGHT_ARROW);
      input_snap_item->sequencer_number = this->sequencer_number;
      input_snap_item->module = module;
      menu->addChild(input_snap_item);

      SampleAndHoldItem *sample_and_hold_item = createMenuItem<SampleAndHoldItem>("Sample & Hold", CHECKMARK(module->voltage_sequencers[sequencer_number].sample_and_hold));
      sample_and_hold_item->sequencer_number = this->sequencer_number;
      sample_and_hold_item->module = module;
      menu->addChild(sample_and_hold_item);
      */

      return menu;
    }
  };

  void appendContextMenu(Menu *menu) override
  {
    DigitalSequencer *module = dynamic_cast<DigitalSequencer *>(this->module);
    assert(module);

    // Menu in development

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Sequencer Settings"));

    // Add "all" sequencer settings
    /* TODO: restore this
    AllSequencersItem *all_sequencer_items;
    all_sequencer_items = createMenuItem<AllSequencersItem>("All Sequencers", RIGHT_ARROW);
    all_sequencer_items->module = module;
    menu->addChild(all_sequencer_items);
    */

    // Add individual sequencer settings
    SequencerItem *sequencer_items[6];

    for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      sequencer_items[i] = createMenuItem<SequencerItem>("Sequencer #" + std::to_string(i + 1), RIGHT_ARROW);
      sequencer_items[i]->module = module;
      sequencer_items[i]->sequencer_number = i;
      menu->addChild(sequencer_items[i]);
    }

    // Reset behavior
    ResetModeItem *reset_mode_item = createMenuItem<ResetModeItem>("Reset Mode", RIGHT_ARROW);
    reset_mode_item->module = module;
    menu->addChild(reset_mode_item);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuItem<QuickKeyMenu>("Quick Key Reference", RIGHT_ARROW));
  }

  void step() override
  {
    ModuleWidget::step();
  }

  //
  // Handler for keypresses that affect the entire module
  //
  void onHoverKey(const event::HoverKey &e) override
  {
    // Switch between seuences using the number keys 1-6
    if (e.key >= GLFW_KEY_1 && e.key <= GLFW_KEY_6)
    {

      if (e.action == GLFW_PRESS)
      {
        unsigned int sequencer_number = e.key - 49;

        sequencer_number = clamp(sequencer_number, 0, NUMBER_OF_SEQUENCERS - 1);
        module->selected_sequencer_index = sequencer_number;
        e.consume(this);
      }
    }

    if ((e.key == GLFW_KEY_F) && ((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL)) // F (no ctrl)
    {
      if (e.action == GLFW_PRESS)
      {
        module->frozen = !module->frozen;
        e.consume(this);
      }
    }

    ModuleWidget::onHoverKey(e);
  }
};
