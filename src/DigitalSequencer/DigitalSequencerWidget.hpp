struct DigitalSequencerWidget : VoxglitchModuleWidget
{
  DigitalSequencer* module;
  int copy_sequencer_index = -1;

  DigitalSequencerWidget(DigitalSequencer* module)
  {
    this->module = module;
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_sequencer/digital_sequencer_front_panel.svg")));

    // PNGPanel *png_panel = new PNGPanel("res/digital_sequencer/digital_sequencer_baseplate_smooth.png", 182.88, 128.5);
    PNGPanel *png_panel = new PNGPanel("res/digital_sequencer/digital_sequencer_baseplate.png", 182.88, 128.5);
    addChild(png_panel);

    // Add typography layer
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_sequencer/digital_sequencer_typography.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);

    // Step
    addInput(createInputCentered<VoxglitchInputPort>(Vec(41.827522,290.250732), module, DigitalSequencer::STEP_INPUT));

    // Reset
    addInput(createInputCentered<VoxglitchInputPort>(Vec(41.822453,349.650879), module, DigitalSequencer::RESET_INPUT));

    // 6 step inputs
    addInput(createInputCentered<VoxglitchInputPort>(Vec(102.700012,349.749878), module, DigitalSequencer::SEQUENCER_1_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(140.200043,349.849854), module, DigitalSequencer::SEQUENCER_2_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(177.550018,349.799866), module, DigitalSequencer::SEQUENCER_3_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(214.900024,349.799805), module, DigitalSequencer::SEQUENCER_4_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(252.350006,349.799866), module, DigitalSequencer::SEQUENCER_5_STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(289.549988,349.849915), module, DigitalSequencer::SEQUENCER_6_STEP_INPUT));

    // step length attenuators
    auto L1 = createParamCentered<VoxglitchAttenuator>(Vec(102.700012, 311.750000), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 0); dynamic_cast<Knob*>(L1)->snap = true; addParam(L1);
    auto L2 = createParamCentered<VoxglitchAttenuator>(Vec(140.200043, 311.750000), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 1); dynamic_cast<Knob*>(L2)->snap = true; addParam(L2);
    auto L3 = createParamCentered<VoxglitchAttenuator>(Vec(177.550018, 311.750000), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 2); dynamic_cast<Knob*>(L3)->snap = true; addParam(L3);
    auto L4 = createParamCentered<VoxglitchAttenuator>(Vec(214.900024, 311.750000), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 3); dynamic_cast<Knob*>(L4)->snap = true; addParam(L4);
    auto L5 = createParamCentered<VoxglitchAttenuator>(Vec(252.350006, 311.750000), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 4); dynamic_cast<Knob*>(L5)->snap = true; addParam(L5);
    auto L6 = createParamCentered<VoxglitchAttenuator>(Vec(289.549988, 311.750000), module, DigitalSequencer::SEQUENCER_LENGTH_KNOBS + 5); dynamic_cast<Knob*>(L6)->snap = true; addParam(L6);

    float sequencer_selection_buttons_x[NUMBER_OF_SEQUENCERS] = {
      102.700012,
      140.200043,
      177.550018,
      214.900024,
      252.350006,
      289.549988
    };

    // Sequence selection buttons
    for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
    {
      addParam(createParamCentered<VoxglitchRoundLampSwitch>(Vec(sequencer_selection_buttons_x[i], 280.250000), module, DigitalSequencer::SEQUENCER_SELECTION_BUTTONS + i));
    }

    // CV outputs
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(359.299927,311.749878), module, DigitalSequencer::SEQ1_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(389.449951,311.749878), module, DigitalSequencer::SEQ2_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(419.799988,311.749878), module, DigitalSequencer::SEQ3_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(450.049988,311.749878), module, DigitalSequencer::SEQ4_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(480.449951,311.749878), module, DigitalSequencer::SEQ5_CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(510.599976,311.749878), module, DigitalSequencer::SEQ6_CV_OUTPUT));

    // Gate outputs
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(359.299927,349.699890), module, DigitalSequencer::SEQ1_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(389.449951, 349.699890), module, DigitalSequencer::SEQ2_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(419.799988, 349.699890), module, DigitalSequencer::SEQ3_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(450.049988, 349.699890), module, DigitalSequencer::SEQ4_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(480.449951, 349.699890), module, DigitalSequencer::SEQ5_GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(510.599976, 349.699890), module, DigitalSequencer::SEQ6_GATE_OUTPUT));

    VoltageSequencerDisplay *voltage_sequencer_display = new VoltageSequencerDisplay();
    voltage_sequencer_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    voltage_sequencer_display->module = module;
    addChild(voltage_sequencer_display);

    for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
    {
      LengthDisplay *length_display = new LengthDisplay();
      length_display->sequencer_number = i;
      length_display->box.pos = Vec(sequencer_selection_buttons_x[i], 331.0);
      length_display->module = module;
      addChild(length_display);
    }
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

      if(module)
      {
        text_to_display = std::to_string(module->voltage_sequencers[sequencer_number].sequence_length);
      }

      std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
    	if (font) {
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

  // Sample and Hold values
  struct SampleAndHoldItem : MenuItem {
    DigitalSequencer *module;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override {
      module->voltage_sequencers[sequencer_number].sample_and_hold ^= true; // flip the value
    }
  };

  //
  // INPUT SNAP MENUS
  //

  struct InputSnapValueItem : MenuItem {
    DigitalSequencer *module;
    int snap_division_index = 0;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override {
      module->voltage_sequencers[sequencer_number].snap_division_index = snap_division_index;
    }
  };

  struct InputSnapItem : MenuItem {
    DigitalSequencer *module;
    int sequencer_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      for (unsigned int i=0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
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

  //
  // OUTPUT RANGE MENUS
  //

  struct OutputRangeValueItem : MenuItem {
    DigitalSequencer *module;
    int range_index = 0;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override {
      module->voltage_sequencers[sequencer_number].voltage_range_index = range_index;
    }
  };

  struct OutputRangeItem : MenuItem {
    DigitalSequencer *module;
    int sequencer_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      for (unsigned int i=0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
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

  struct SequencerItem : MenuItem {
    DigitalSequencer *module;
    unsigned int sequencer_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

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

      return menu;
    }
  };

  struct ResetOnNextOption : MenuItem {
    DigitalSequencer *module;

    void onAction(const event::Action &e) override {
      module->legacy_reset = false;
    }
  };

  struct ResetInstantOption : MenuItem {
    DigitalSequencer *module;

    void onAction(const event::Action &e) override {
      module->legacy_reset = true;
    }
  };

  struct ResetModeItem : MenuItem {
    DigitalSequencer *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      ResetOnNextOption *reset_on_next = createMenuItem<ResetOnNextOption>("Next clock input.", CHECKMARK(! module->legacy_reset));
      reset_on_next->module = module;
      menu->addChild(reset_on_next);

      ResetInstantOption *reset_instant = createMenuItem<ResetInstantOption>("Instant", CHECKMARK(module->legacy_reset));
      reset_instant->module = module;
      menu->addChild(reset_instant);

      return menu;
    }
  };

  struct QuickKeyMenu : MenuItem {
    Menu *createChildMenu() override {
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

  struct AllSequencersItem : MenuItem {

  };

  void appendContextMenu(Menu *menu) override
  {
    DigitalSequencer *module = dynamic_cast<DigitalSequencer*>(this->module);
    assert(module);

    // Menu in development

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Sequencer Settings"));

    // Add "all" sequencer settings
    /*
    AllSequencersItem *all_sequencer_items;
    all_sequencer_items = createMenuItem<AllSequencersItem>("All Sequencers", RIGHT_ARROW);
    menu->addChild(all_sequencer_items);
    */

    // Add individual sequencer settings
    SequencerItem *sequencer_items[6];

    for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      sequencer_items[i] = createMenuItem<SequencerItem>("Sequencer #" + std::to_string(i + 1), RIGHT_ARROW);
      sequencer_items[i]->module = module;
      sequencer_items[i]->sequencer_number = i;
      menu->addChild(sequencer_items[i]);
    }

    // Reset behavior
    /*
    LegacyResetOption *legacy_reset_option = createMenuItem<LegacyResetOption>("Legacy Reset", CHECKMARK(module->legacy_reset));
    legacy_reset_option->module = module;
    menu->addChild(legacy_reset_option);
    */

    ResetModeItem *reset_mode_item = createMenuItem<ResetModeItem>("Reset Mode", RIGHT_ARROW);
    reset_mode_item->module = module;
    menu->addChild(reset_mode_item);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuItem<QuickKeyMenu>("Quick Key Reference", RIGHT_ARROW));
  }

  void step() override {
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

        if(e.action == GLFW_PRESS)
        {
          unsigned int sequencer_number = e.key - 49;

          // DEBUG(std::to_string(sequencer_number).c_str());
          sequencer_number = clamp(sequencer_number,0,NUMBER_OF_SEQUENCERS-1);
          module->selected_sequencer_index = sequencer_number;
          e.consume(this);
        }

      }

      if ((e.key == GLFW_KEY_F) && ((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL)) // F (no ctrl)
      {
        if(e.action == GLFW_PRESS)
        {
          module->frozen = ! module->frozen;
          e.consume(this);
        }
      }


      if ((e.key == GLFW_KEY_C) && ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL)) // Control-C
      {
        if(e.action == GLFW_PRESS)
        {
          copy_sequencer_index = module->selected_sequencer_index;
          e.consume(this);
        }
      }

      if ((e.key == GLFW_KEY_V) && ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL)) // Control-V
      {
        if(e.action == GLFW_PRESS)
        {
          if(copy_sequencer_index > -1)
          {
            module->copy(copy_sequencer_index, module->selected_sequencer_index);
            e.consume(this);
          }
        }
      }

      #ifdef DEV_MODE
        if(e.action == GLFW_PRESS && e.key == GLFW_KEY_P)
        {
          std::string debug_string = "mouse at: " + std::to_string(e.pos.x) + "," + std::to_string(e.pos.y);
          DEBUG(debug_string.c_str());
        }
        ModuleWidget::onHoverKey(e);
      #endif

      ModuleWidget::onHoverKey(e);

      // module->selected_voltage_sequencer->shiftRight();
      // if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->shiftRight();
  }
};
