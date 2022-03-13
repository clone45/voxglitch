struct WavBankWidget : ModuleWidget
{
	WavBankWidget(WavBank* module)
	{
		setModule(module);

    // Set the background SVG panel.  This should be blank
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/wavbank/wav_bank_panel_container.svg")));


    // Load up the background PNG and add it to the panel
    PNGPanel *png_panel = new PNGPanel("res/wavbank/wav_bank_base.png", 50.8, 128.5);
    addChild(png_panel);

    // Add typography layer
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/wavbank/wav_bank_typography.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);

    addParam(createParamCentered<VoxglitchLargeKnob>(mm2px(Vec(16.544, 26.185)), module, WavBank::WAV_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(mm2px(Vec(16.544, 46.09)), module, WavBank::WAV_ATTN_KNOB));

		addParam(createParamCentered<squareToggle>(mm2px(Vec(40.72, 96.12)), module, WavBank::LOOP_SWITCH));

    WavBankReadout *readout = new WavBankReadout();
		readout->box.pos = mm2px(Vec(40.68, 78));
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// Input jacks
		addInput(createInputCentered<VoxglitchInputPort>(Vec(48.550400,227.734650), module, WavBank::TRIG_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(48.550400,171.650101), module, WavBank::WAV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(48.732090,283.751099), module, WavBank::PITCH_INPUT));

		// WAV output
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(74.990524,349.837158), module, WavBank::WAV_LEFT_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(120.190491,349.837158), module, WavBank::WAV_RIGHT_OUTPUT));

	}

  //
  // menu structure for selecting between different trigger input behaviors
  //

  struct TriggerOption : MenuItem {
    WavBank *module;

    void onAction(const event::Action &e) override {
      module->trig_input_response_mode = TRIGGER;
    }
  };

  struct GateOption : MenuItem {
    WavBank *module;

    void onAction(const event::Action &e) override {
    module->trig_input_response_mode = GATE;
    }
  };

  struct TriggerModeMenu : MenuItem {
    WavBank *module;

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
		WavBank *module = dynamic_cast<WavBank*>(this->module);
		assert(module);

		// For spacing only
		menu->addChild(new MenuEntry);

    TriggerModeMenu *trigger_mode_menu = createMenuItem<TriggerModeMenu>("Trigger Mode", RIGHT_ARROW);
    trigger_mode_menu->module = module;
    menu->addChild(trigger_mode_menu);

		// Add the "Select Directory Containing WAV Files" menu item
		MenuItemLoadBank *menu_item_load_bank = new MenuItemLoadBank();
		menu_item_load_bank->text = "Select Directory Containing WAV Files";
		menu_item_load_bank->wav_bank_module = module;
		menu->addChild(menu_item_load_bank);
	}

};
