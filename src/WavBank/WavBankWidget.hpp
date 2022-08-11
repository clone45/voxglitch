struct WavBankWidget : VoxglitchSamplerModuleWidget
{
	WavBankWidget(WavBank* module)
	{
		setModule(module);

    // Set the background SVG panel.  This should be blank
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/wavbank/wav_bank_panel_container.svg")));

    // Load up the background PNG and add it to the panel
    PNGPanel *png_panel = new PNGPanel("res/wavbank/wav_bank_base_small.png", 50.8, 128.5);
    addChild(png_panel);

    // Add typography layer

    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/wavbank/wav_bank_typography.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);

    addParam(createParamCentered<VoxglitchLargeKnob>(Vec(48.800396,78.217896), module, WavBank::WAV_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(112.900337,60.692520), module, WavBank::WAV_ATTN_KNOB));

		addParam(createParamCentered<squareToggle>(Vec(120.186218,280.068604), module, WavBank::LOOP_SWITCH));

    WavBankReadout *readout = new WavBankReadout();
		readout->box.pos = Vec(13.300354,141);
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// Input jacks
		addInput(createInputCentered<VoxglitchInputPort>(Vec(29.850380,280.335236), module, WavBank::TRIG_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(112.800354,97.550163), module, WavBank::WAV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(75.182068,280.500671), module, WavBank::PITCH_INPUT));

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
		menu_item_load_bank->module = module;
		menu->addChild(menu_item_load_bank);

    // Sample interpolation settings
    menu->addChild(new MenuEntry); // For spacing only
    SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
    sample_interpolation_menu_item->module = module;
    menu->addChild(sample_interpolation_menu_item);
	}

};
