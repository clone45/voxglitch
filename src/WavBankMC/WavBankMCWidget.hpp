struct WavBankMCWidget : VoxglitchModuleWidget
{
	WavBankMCWidget(WavBankMC* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/wavbank_mc/wavbank_mc_front_panel.svg")));

    // Load up the background PNG and add it to the panel
    PNGPanel *png_panel = new PNGPanel("res/wavbank_mc/wavbank_mc_base.png", 101.6, 128.5);
    addChild(png_panel);

    // Add typography layer
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/wavbank_mc/wavbank_mc_typography.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);

    addParam(createParamCentered<VoxglitchLargeKnob>(Vec(202.096863,80.255463), module, WavBankMC::WAV_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(270.199890,97.450043), module, WavBankMC::WAV_INPUT));
    // addParam(createParamCentered<Trimpot>(mm2px(Vec(COL18, ROW10)), module, WavBankMC::WAV_ATTN_KNOB));

    addInput(createInputCentered<VoxglitchInputPort>(Vec(179.650024,160.600006), module, WavBankMC::TRIG_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(225.000000,160.600006), module, WavBankMC::NEXT_WAV_TRIGGER_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(270.250000,160.600006), module, WavBankMC::PREV_WAV_TRIGGER_INPUT));

    // TODO: Think of being able to light up the light without pressing the button
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(179.757858,189.357864), module, WavBankMC::TRIG_INPUT_BUTTON_PARAM));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(225.100021,189.357864), module, WavBankMC::NEXT_WAV_BUTTON_PARAM));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(270.250000,189.357864), module, WavBankMC::PREV_WAV_BUTTON_PARAM));


    addInput(createInputCentered<VoxglitchInputPort>(Vec(179.650055,240.359680), module, WavBankMC::PITCH_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(225.000061,240.248474), module, WavBankMC::VOLUME_INPUT));

		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(270.199677,240.283936), module, WavBankMC::POLY_WAV_OUTPUT));

    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(225.000000,349.589600), module, WavBankMC::LEFT_WAV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(270.250549,349.739563), module, WavBankMC::RIGHT_WAV_OUTPUT));

		addParam(createParamCentered<squareToggle>(Vec(173.499985,350.189453), module, WavBankMC::LOOP_SWITCH));

    addParam(createParamCentered<VoxglitchAttenuator>(Vec(270.299652,60.532650), module, WavBankMC::WAV_ATTN_KNOB));

    WavBankMCReadout *readout = new WavBankMCReadout();
		readout->box.pos = mm2px(Vec(8, 8));
		readout->box.size = Vec(READOUT_WIDTH, READOUT_HEIGHT); // bounding box of the widget
		readout->module = module;
		addChild(readout);


    // add grime layer
    GrimeLayer *grime_layer = new GrimeLayer(0, 0, 130.21875, 343.25);
    grime_layer->box.pos = Vec(18.250000, 18.250000);
    addChild(grime_layer);


    //

		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
/*

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COL18, ROW19 + 1.0)), module, WavBankMC::PITCH_INPUT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(COL18, ROW29)), module, WavBankMC::LOOP_SWITCH));

    // Next wav button and input

		addParam(createParamCentered<LEDButton>(mm2px(Vec(COL28, ROW12)), module, WavBankMC::NEXT_WAV_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(COL28, ROW12)), module, WavBankMC::NEXT_WAV_LIGHT));

    // Prev wav button and input

		addParam(createParamCentered<LEDButton>(mm2px(Vec(COL28, ROW17)), module, WavBankMC::PREV_WAV_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(COL28, ROW17)), module, WavBankMC::PREV_WAV_LIGHT));

		WavBankMCReadout *readout = new WavBankMCReadout();
		readout->box.pos = mm2px(Vec(4, 3));
		readout->box.size = Vec(READOUT_WIDTH, READOUT_HEIGHT); // bounding box of the widget
		readout->module = module;
		addChild(readout);
    */
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
		menu_item_load_bank_mc->wav_bank_mc_module = module;
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
