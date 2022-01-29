/*
class TrimpotNoRandom : public Trimpot
{
public:
  void randomize() override {} // do nothing. base class would actually randomize
};
*/

struct DigitalSequencerXPWidget : ModuleWidget
{
  DigitalSequencerXP* module;
  int copy_sequencer_index = -1;

  DigitalSequencerXPWidget(DigitalSequencerXP* module)
  {
    this->module = module;
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_sequencer_xp_front_panel.svg")));

    // Cosmetic rack screws
    addChild(createWidget<ScrewSilver>(Vec(15, 365)));
    addChild(createWidget<ScrewSilver>(mm2px(Vec(171.5, 0))));

    // Main voltage sequencer display
    VoltageSequencerDisplayXP *voltage_sequencer_display_xp = new VoltageSequencerDisplayXP();
    voltage_sequencer_display_xp->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    voltage_sequencer_display_xp->module = module;

    GateSequencerDisplayXP *gates_display = new GateSequencerDisplayXP();
    gates_display->box.pos = mm2px(Vec(GATES_DRAW_AREA_POSITION_X, GATES_DRAW_AREA_POSITION_Y));
    gates_display->module = module;

    addChild(voltage_sequencer_display_xp);
    addChild(gates_display);

    double button_spacing = 9.6; // 9.1
    double vertical_button_spacing = 8;
    double button_group_x = 48.0;
    double button_group_y = 103.0;


    // Draw sequence buttons
    int half_number_of_sequencers = NUMBER_OF_SEQUENCERS/2;

    for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
    {
      float x_position = (button_group_x + (button_spacing * (i % half_number_of_sequencers)));
      float y_position = button_group_y + (i/half_number_of_sequencers * vertical_button_spacing);
      addParam(createParamCentered<LEDButton>(mm2px(Vec(x_position, y_position)), module, DigitalSequencerXP::SEQUENCER_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(x_position, y_position)), module, DigitalSequencerXP::SEQUENCER_LIGHTS + i));
    }

    // addParam(createParamCentered<Trimpot>(mm2px(Vec(button_group_x, button_group_y + 8.6)), module, DigitalSequencerXP::POLY_SEQUENCER_LENGTH_INPUT));

    // addParam(createParamCentered<Trimpot>(mm2px(Vec(button_group_x, button_group_y + 8.6)), module, DigitalSequencerXP::SEQUENCER_1_LENGTH_KNOB));

    // Inputs
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 114.893)), module, DigitalSequencerXP::POLY_STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 14.544, 114.893)), module, DigitalSequencerXP::POLY_LENGTH_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 29.088, 114.893)), module, DigitalSequencerXP::RESET_INPUT));

    // Poly CV and Gate outputs
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(173, 107.568)), module, DigitalSequencerXP::POLY_CV_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(173, 118.653)), module, DigitalSequencerXP::POLY_GATE_OUTPUT));

    // addParam(createParamCentered<FreezeToggle>(mm2px(Vec(180,40)), module, DigitalSequencerXP::FREEZE_TOGGLE));
  }

  /*
  // Sample and Hold values
  struct SampleAndHoldItem : MenuItem {
    DigitalSequencerXP *module;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override {
      module->voltage_sequencers[sequencer_number].sample_and_hold ^= true; // flip the value
    }
  };

  //
  // INPUT SNAP MENUS
  //

  struct InputSnapValueItem : MenuItem {
    DigitalSequencerXP *module;
    int snap_division_index = 0;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override {
      module->voltage_sequencers[sequencer_number].snap_division_index = snap_division_index;
    }
  };

  struct InputSnapItem : MenuItem {
    DigitalSequencerXP *module;
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
    DigitalSequencerXP *module;
    int range_index = 0;
    int sequencer_number = 0;

    void onAction(const event::Action &e) override {
      module->voltage_sequencers[sequencer_number].voltage_range_index = range_index;
    }
  };

  struct OutputRangeItem : MenuItem {
    DigitalSequencerXP *module;
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
    DigitalSequencerXP *module;
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
    DigitalSequencerXP *module;

    void onAction(const event::Action &e) override {
      module->legacy_reset = false;
    }
  };

  struct ResetInstantOption : MenuItem {
    DigitalSequencerXP *module;

    void onAction(const event::Action &e) override {
      module->legacy_reset = true;
    }
  };

  struct ResetModeItem : MenuItem {
    DigitalSequencerXP *module;

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
    DigitalSequencerXP *module = dynamic_cast<DigitalSequencerXP*>(this->module);
    assert(module);

    // Menu in development

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Sequencer Settings"));

    // Add individual sequencer settings
    SequencerItem *sequencer_items[6];

    for(unsigned int i=0; i < NUMBER_OF_SEQUENCERS; i++)
    {
      sequencer_items[i] = createMenuItem<SequencerItem>("Sequencer #" + std::to_string(i + 1), RIGHT_ARROW);
      sequencer_items[i]->module = module;
      sequencer_items[i]->sequencer_number = i;
      menu->addChild(sequencer_items[i]);
    }

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

      ModuleWidget::onHoverKey(e);

      // module->selected_voltage_sequencer->shiftRight();
      // if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->shiftRight();

  }
  */
};
