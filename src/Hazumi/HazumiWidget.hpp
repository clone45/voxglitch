struct HazumiWidget : ModuleWidget
{
  float gate_output_locations[SEQUENCER_COLUMNS] = {
    48.1875, 58.226, 68.2656, 78.304, 88.34375, 98.3828, 108.4218, 118.461
  };

  HazumiWidget(Hazumi* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hazumi_front_panel.svg")));

    // Step & Reset inputs
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(75.25, 12.0468)), module, Hazumi::STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(75.25, 29.617)), module, Hazumi::RESET_INPUT));

    // Add outputs
    for(unsigned int i=0; i<SEQUENCER_COLUMNS; i++)
    {
      float x = 75.25;
      float y = gate_output_locations[i];
      addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x, y)), module, Hazumi::GATE_OUTPUTS + i));
    }

    // Add display
    HazumiSequencerDisplay *hazumi_sequencer_display = new HazumiSequencerDisplay();
    hazumi_sequencer_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    hazumi_sequencer_display->module = module;
    addChild(hazumi_sequencer_display);
  }

  struct TriggerOptionValueItem : MenuItem {
    Hazumi *module;
    int option = 0;
    int column = 0;

    void onAction(const event::Action &e) override {
      if(column >= 0)
      {
        module->hazumi_sequencer.trigger_options[column] = option;
      }
      else
      {
        for(unsigned int i=0; i < SEQUENCER_COLUMNS; i++)
        {
          module->hazumi_sequencer.trigger_options[i] = option;
        }
      }
    }
  };

  struct TriggerOptionMenuItem : MenuItem {
    Hazumi *module;
    int column = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      // trigger location == 0 (bottom) is default
      TriggerOptionValueItem *trigger_option_value_item_0 = createMenuItem<TriggerOptionValueItem>(module->trigger_options_names[TRIGGER_AT_BOTTOM], CHECKMARK(module->hazumi_sequencer.trigger_options[this->column] == 0));
      trigger_option_value_item_0->module = module;
      trigger_option_value_item_0->option = TRIGGER_AT_BOTTOM;
      trigger_option_value_item_0->column = this->column;
      menu->addChild(trigger_option_value_item_0);

      TriggerOptionValueItem *trigger_option_value_item_1 = createMenuItem<TriggerOptionValueItem>(module->trigger_options_names[TRIGGER_AT_TOP], CHECKMARK(module->hazumi_sequencer.trigger_options[this->column] == 1));
      trigger_option_value_item_1->module = module;
      trigger_option_value_item_1->option = TRIGGER_AT_TOP;
      trigger_option_value_item_1->column = this->column;
      menu->addChild(trigger_option_value_item_1);

      TriggerOptionValueItem *trigger_option_value_item_2 = createMenuItem<TriggerOptionValueItem>(module->trigger_options_names[TRIGGER_AT_BOTH], CHECKMARK(module->hazumi_sequencer.trigger_options[this->column] == 2));
      trigger_option_value_item_2->module = module;
      trigger_option_value_item_2->option = TRIGGER_AT_BOTH;
      trigger_option_value_item_2->column = this->column;
      menu->addChild(trigger_option_value_item_2);

      return menu;
    }
  };

  struct SequencerItemAll : MenuItem {
    Hazumi *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      TriggerOptionMenuItem *trigger_option_menu_item = createMenuItem<TriggerOptionMenuItem>("Trigger Location", RIGHT_ARROW);
      trigger_option_menu_item->column = -1; // -1 means "all" in this context
      trigger_option_menu_item->module = module;
      menu->addChild(trigger_option_menu_item);

      return menu;
    }
  };

  struct SequencerItem : MenuItem {
    Hazumi *module;
    unsigned int column = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      TriggerOptionMenuItem *trigger_option_menu_item = createMenuItem<TriggerOptionMenuItem>("Trigger Location", RIGHT_ARROW);
      trigger_option_menu_item->column = this->column;
      trigger_option_menu_item->module = module;
      menu->addChild(trigger_option_menu_item);

      return menu;
    }
  };

  void appendContextMenu(Menu *menu) override
  {
    Hazumi *module = dynamic_cast<Hazumi*>(this->module);
    assert(module);

    // Menu in development
    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Column Settings"));

    SequencerItem *all_sequencer_items = createMenuItem<SequencerItem>("All Columns", RIGHT_ARROW);
    all_sequencer_items->module = module;
    all_sequencer_items->column = -1; // -1 means "all columns"
    menu->addChild(all_sequencer_items);

    // Add individual sequencer settings
    SequencerItem *sequencer_items[8];

    for(unsigned int i=0; i < SEQUENCER_COLUMNS; i++)
    {
      sequencer_items[i] = createMenuItem<SequencerItem>("Column #" + std::to_string(i + 1), RIGHT_ARROW);
      sequencer_items[i]->module = module;
      sequencer_items[i]->column = i;
      menu->addChild(sequencer_items[i]);
    }
  }

};
