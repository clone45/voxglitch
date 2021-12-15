/*
class TrimpotNoRandom : public Trimpot
{
public:
  void randomize() override {} // do nothing. base class would actually randomize
};
*/

/* Abandoning this front-panel control for now
struct FreezeToggle : app::SvgSwitch {
	FreezeToggle() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/freeze-button-off.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/freeze-button-on.svg")));
	};
};
*/

struct DigitalSequencerWidget : ModuleWidget
{
  DigitalSequencer* module;
  int copy_sequencer_index = -1;

  DigitalSequencerWidget(DigitalSequencer* module)
  {
    this->module = module;
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_sequencer_front_panel.svg")));

    // Cosmetic rack screws
    // addChild(createWidget<ScrewSilver>(Vec(15, 0)));
    addChild(createWidget<ScrewSilver>(Vec(15, 365)));
    addChild(createWidget<ScrewSilver>(mm2px(Vec(171.5, 0))));

    // Main voltage sequencer display
    VoltageSequencerDisplay *voltage_sequencer_display = new VoltageSequencerDisplay();
    voltage_sequencer_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    voltage_sequencer_display->module = module;
    addChild(voltage_sequencer_display);

    GateSequencerDisplay *gates_display = new GateSequencerDisplay();
    gates_display->box.pos = mm2px(Vec(GATES_DRAW_AREA_POSITION_X, GATES_DRAW_AREA_POSITION_Y));
    gates_display->module = module;
    addChild(gates_display);

    double button_spacing = 9.6; // 9.1
    double button_group_x = 48.0;
    double button_group_y = 103.0;
    // Sequence 1 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x, button_group_y)), module, DigitalSequencer::SEQUENCER_1_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x, button_group_y)), module, DigitalSequencer::SEQUENCER_1_LIGHT));
    // Sequence 2 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, DigitalSequencer::SEQUENCER_2_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, DigitalSequencer::SEQUENCER_2_LIGHT));
    // Sequence 3 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, DigitalSequencer::SEQUENCER_3_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, DigitalSequencer::SEQUENCER_3_LIGHT));
    // Sequence 4 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, DigitalSequencer::SEQUENCER_4_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, DigitalSequencer::SEQUENCER_4_LIGHT));
    // Sequence 5 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, DigitalSequencer::SEQUENCER_5_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, DigitalSequencer::SEQUENCER_5_LIGHT));
    // Sequence 6 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y)), module, DigitalSequencer::SEQUENCER_6_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y)), module, DigitalSequencer::SEQUENCER_6_LIGHT));

    // addParam(createParamCentered<Trimpot>(mm2px(Vec(button_group_x, button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_1_LENGTH_KNOB));

    auto L1 = createParamCentered<Trimpot>(mm2px(Vec(button_group_x, button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_1_LENGTH_KNOB); dynamic_cast<Knob*>(L1)->snap = true; addParam(L1);
    auto L2 = createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_2_LENGTH_KNOB); dynamic_cast<Knob*>(L2)->snap = true; addParam(L2);
    auto L3 = createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_3_LENGTH_KNOB); dynamic_cast<Knob*>(L3)->snap = true; addParam(L3);
    auto L4 = createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_4_LENGTH_KNOB); dynamic_cast<Knob*>(L4)->snap = true; addParam(L4);
    auto L5 = createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_5_LENGTH_KNOB); dynamic_cast<Knob*>(L5)->snap = true; addParam(L5);
    auto L6 = createParamCentered<Trimpot>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y + 8.6)), module, DigitalSequencer::SEQUENCER_6_LENGTH_KNOB); dynamic_cast<Knob*>(L6)->snap = true; addParam(L6);

    // 6 step inputs
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x, button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_1_STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_2_STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_3_STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_4_STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_5_STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 5.0), button_group_y + 18.0)), module, DigitalSequencer::SEQUENCER_6_STEP_INPUT));

    // Step
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 114.893)), module, DigitalSequencer::STEP_INPUT));

    // Reset
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 14.544, 114.893)), module, DigitalSequencer::RESET_INPUT));

    // 6 sequencer outputs
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(118, 108.224)), module, DigitalSequencer::SEQ1_CV_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(129, 108.224)), module, DigitalSequencer::SEQ2_CV_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(140, 108.224)), module, DigitalSequencer::SEQ3_CV_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(151, 108.224)), module, DigitalSequencer::SEQ4_CV_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(162, 108.224)), module, DigitalSequencer::SEQ5_CV_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(173, 108.224)), module, DigitalSequencer::SEQ6_CV_OUTPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(118, 119.309)), module, DigitalSequencer::SEQ1_GATE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(129, 119.309)), module, DigitalSequencer::SEQ2_GATE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(140, 119.309)), module, DigitalSequencer::SEQ3_GATE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(151, 119.309)), module, DigitalSequencer::SEQ4_GATE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(162, 119.309)), module, DigitalSequencer::SEQ5_GATE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(173, 119.309)), module, DigitalSequencer::SEQ6_GATE_OUTPUT));

    // addParam(createParamCentered<FreezeToggle>(mm2px(Vec(180,40)), module, DigitalSequencer::FREEZE_TOGGLE));
  }


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
    menu->addChild(createMenuItem<QuickKeyMenu>("Quick Key Reference", RIGHT_ARROW));
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

      ModuleWidget::onHoverKey(e);

      // module->selected_voltage_sequencer->shiftRight();
      // if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->shiftRight();
  }
};
