struct WavBankMCWidget : ModuleWidget
{
	WavBankMCWidget(WavBankMC* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/wav_bank_mc_front_panel.svg")));

		// Cosmetic rack screws
		// addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		// addChild(createWidget<ScrewSilver>(Vec(15, 365)));


		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COL10, ROW7)), module, WavBankMC::TRIG_INPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COL10, ROW12)), module, WavBankMC::WAV_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(COL10, ROW15)), module, WavBankMC::WAV_ATTN_KNOB));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(COL10, ROW19)), module, WavBankMC::WAV_KNOB));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COL14, ROW7)), module, WavBankMC::VOLUME_INPUT));

		addParam(createParamCentered<CKSS>(mm2px(Vec(COL10, ROW24)), module, WavBankMC::LOOP_SWITCH));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COL10, ROW29)), module, WavBankMC::PITCH_INPUT));

    // Next wav button and input
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COL14, ROW19)), module, WavBankMC::NEXT_WAV_TRIGGER_INPUT));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(COL14, ROW22)), module, WavBankMC::NEXT_WAV_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(COL14, ROW22)), module, WavBankMC::NEXT_WAV_LIGHT));

    // Prev wav button and input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COL14, ROW12)), module, WavBankMC::PREV_WAV_TRIGGER_INPUT));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(COL14, ROW15)), module, WavBankMC::PREV_WAV_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(COL14, ROW15)), module, WavBankMC::PREV_WAV_LIGHT));

		WavBankMCReadout *readout = new WavBankMCReadout();
		readout->box.pos = mm2px(Vec(4, ROW1));
		readout->box.size = Vec(110, 50); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// WAV output
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 104)), module, WavBankMC::WAV_LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COL14, ROW29)), module, WavBankMC::POLY_WAV_OUTPUT));
	}

  // SampleChangeMode on sample change
  /*
  struct SampleChangeModeMenuItem : MenuItem {
    WavBankMC* module;
    void onAction(const event::Action& e) override {
      module->sample_change_mode = !(module->sample_change_mode);
    }
  };
  */

  struct RestartOption : MenuItem {
    WavBankMC *module;
    void onAction(const event::Action &e) override {
      module->sample_change_mode = RESTART_PLAYBACK;
    }
  };

  struct ContinualOption : MenuItem {
    WavBankMC *module;
    void onAction(const event::Action &e) override {
      module->sample_change_mode = CONTINUAL_PLAYBACK;
    }
  };

  struct SampleChangeModeMenu : MenuItem {
    WavBankMC *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      RestartOption *restart_option = createMenuItem<RestartOption>("Restart", CHECKMARK(module->sample_change_mode == RESTART_PLAYBACK));
      restart_option->module = module;
      menu->addChild(restart_option);

      ContinualOption *continual_option = createMenuItem<ContinualOption>("Continual", CHECKMARK(module->sample_change_mode == CONTINUAL_PLAYBACK));
      continual_option->module = module;
      menu->addChild(continual_option);

      return menu;
    }
  };

  struct SmoothingMenuItem : MenuItem {
    WavBankMC* module;
    void onAction(const event::Action& e) override {
      module->smoothing = !(module->smoothing);
    }
  };



  //
  // menu structure for selecting between different trigger input behaviors
  //
  /*
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
  */
  // }} End of trigger mode menu code


	void appendContextMenu(Menu *menu) override
	{
		WavBankMC *module = dynamic_cast<WavBankMC*>(this->module);
		assert(module);

		// For spacing only
		menu->addChild(new MenuEntry);

    /*
    TriggerModeMenu *trigger_mode_menu = createMenuItem<TriggerModeMenu>("Trigger Mode", RIGHT_ARROW);
    trigger_mode_menu->module = module;
    menu->addChild(trigger_mode_menu);
    */

		SampleChangeModeMenu* sample_change_mode_menu = createMenuItem<SampleChangeModeMenu>("Sample Change Behavior", RIGHT_ARROW);
		sample_change_mode_menu->module = module;
		menu->addChild(sample_change_mode_menu);

    SmoothingMenuItem* smoothing_menu_item = createMenuItem<SmoothingMenuItem>("Smoothing");
    smoothing_menu_item->rightText = CHECKMARK(module->smoothing == 1);
    smoothing_menu_item->module = module;
    menu->addChild(smoothing_menu_item);

		// Add the "Select Directory Containing WAV Files" menu item
		MenuItemLoadBankMC *menu_item_load_bank_mc = new MenuItemLoadBankMC();
		menu_item_load_bank_mc->text = "Select Directory Containing WAV Files";
		menu_item_load_bank_mc->wav_bank_mc_module = module;
		menu->addChild(menu_item_load_bank_mc);
	}

};
