struct WavBankMCWidget : VoxglitchModuleWidget
{
	WavBankMCWidget(WavBankMC* module)
	{
		setModule(module);

    // Load and apply theme
    theme.load("wavbank_mc");
    applyTheme();

    addParam(createParamCentered<VoxglitchLargeKnob>(themePos("WAV_KNOB"), module, WavBankMC::WAV_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("WAV_INPUT"), module, WavBankMC::WAV_INPUT));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("WAV_ATTN_KNOB"), module, WavBankMC::WAV_ATTN_KNOB));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIG_INPUT"), module, WavBankMC::TRIG_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("NEXT_WAV_TRIGGER_INPUT"), module, WavBankMC::NEXT_WAV_TRIGGER_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("PREV_WAV_TRIGGER_INPUT"), module, WavBankMC::PREV_WAV_TRIGGER_INPUT));

    // TODO: Think of being able to light up the light without pressing the button
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("TRIG_INPUT_BUTTON_PARAM"), module, WavBankMC::TRIG_INPUT_BUTTON_PARAM));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("NEXT_WAV_BUTTON_PARAM"), module, WavBankMC::NEXT_WAV_BUTTON_PARAM));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("PREV_WAV_BUTTON_PARAM"), module, WavBankMC::PREV_WAV_BUTTON_PARAM));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("PITCH_INPUT"), module, WavBankMC::PITCH_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("VOLUME_INPUT"), module, WavBankMC::VOLUME_INPUT));

		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("POLY_WAV_OUTPUT"), module, WavBankMC::POLY_WAV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("LEFT_WAV_OUTPUT"), module, WavBankMC::LEFT_WAV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RIGHT_WAV_OUTPUT"), module, WavBankMC::RIGHT_WAV_OUTPUT));

		addParam(createParamCentered<squareToggle>(themePos("LOOP_SWITCH"), module, WavBankMC::LOOP_SWITCH));

    WavBankMCReadout *readout = new WavBankMCReadout();
		readout->box.pos = mm2px(Vec(8, 8));
		readout->box.size = themePos("READOUT"); // bounding box of the widget
		readout->module = module;
		addChild(readout);
	}


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
		menu_item_load_bank_mc->module = module;
		menu->addChild(menu_item_load_bank_mc);
	}

  #ifdef DEV_MODE
    void onHoverKey(const event::HoverKey &e) override
    {
      if(e.action == GLFW_PRESS && e.key == GLFW_KEY_P)
      {
        std::string debug_string = "mouse at: " + std::to_string(e.pos.x) + "," + std::to_string(e.pos.y);
        DEBUG(debug_string.c_str());
      }
      ModuleWidget::onHoverKey(e);
    }
  #endif

};
