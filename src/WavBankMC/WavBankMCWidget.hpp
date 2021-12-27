struct WavBankMCWidget : ModuleWidget
{
	WavBankMCWidget(WavBankMC* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/wav_bank_mc_front_panel.svg")));

		// Cosmetic rack screws
		// addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		// addChild(createWidget<ScrewSilver>(Vec(15, 365)));

    float column_1 = 13.185;

    float column_2 = 58.905;
    float column_3 = 34.236;
    float column_4 = 79.956;

    float column_b = 10;
    float bottom_row = 104.893;

		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_2, 25.535)), module, WavBankMC::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_2, 46)), module, WavBankMC::WAV_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_2, 114.893)), module, WavBankMC::PITCH_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_4, 46)), module, WavBankMC::VOLUME_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(column_2, 60)), module, WavBankMC::WAV_ATTN_KNOB));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(column_2, 75)), module, WavBankMC::WAV_KNOB));
		addParam(createParamCentered<CKSS>(mm2px(Vec(column_2, 97)), module, WavBankMC::LOOP_SWITCH));

    // Next wav button and input
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_b, bottom_row)), module, WavBankMC::NEXT_WAV_TRIGGER_INPUT));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(column_b, bottom_row + 10)), module, WavBankMC::NEXT_WAV_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(column_b, bottom_row + 10)), module, WavBankMC::NEXT_WAV_LIGHT));

    // Prev wav button and input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_b + 15, bottom_row)), module, WavBankMC::PREV_WAV_TRIGGER_INPUT));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(column_b + 15, bottom_row + 10)), module, WavBankMC::PREV_WAV_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(column_b + 15, bottom_row + 10)), module, WavBankMC::PREV_WAV_LIGHT));

		WavBankMCReadout *readout = new WavBankMCReadout();
		readout->box.pos = mm2px(Vec(2, 10));
		readout->box.size = Vec(110, 50); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// WAV output
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 104)), module, WavBankMC::WAV_LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_4, 114.9)), module, WavBankMC::POLY_WAV_OUTPUT));
	}

  //
  // menu structure for selecting between different trigger input behaviors
  //

  struct TriggerOption : MenuItem {
    WavBankMC *module;

    void onAction(const event::Action &e) override {
      module->trig_input_response_mode = TRIGGER;
    }
  };

  struct GateOption : MenuItem {
    WavBankMC *module;

    void onAction(const event::Action &e) override {
    module->trig_input_response_mode = GATE;
    }
  };

  struct TriggerModeMenu : MenuItem {
    WavBankMC *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      TriggerOption *trigger_option = createMenuItem<TriggerOption>("Trigger", CHECKMARK(module->trig_input_response_mode == TRIGGER));
      trigger_option->module = module;
      menu->addChild(trigger_option);

      GateOption *gate_option = createMenuItem<GateOption>("Gate", CHECKMARK(module->trig_input_response_mode == GATE));
      gate_option->module = module;
      menu->addChild(gate_option);

      return menu;
    }
  };

  // }} End of trigger mode menu code


	void appendContextMenu(Menu *menu) override
	{
		WavBankMC *module = dynamic_cast<WavBankMC*>(this->module);
		assert(module);

		// For spacing only
		menu->addChild(new MenuEntry);

    TriggerModeMenu *trigger_mode_menu = createMenuItem<TriggerModeMenu>("Trigger Mode", RIGHT_ARROW);
    trigger_mode_menu->module = module;
    menu->addChild(trigger_mode_menu);

		// Add the "Select Directory Containing WAV Files" menu item
		MenuItemLoadBankMC *menu_item_load_bank_mc = new MenuItemLoadBankMC();
		menu_item_load_bank_mc->text = "Select Directory Containing WAV Files";
		menu_item_load_bank_mc->wav_bank_mc_module = module;
		menu->addChild(menu_item_load_bank_mc);
	}

};
