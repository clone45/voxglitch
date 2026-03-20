struct OneShotWidget : ModuleWidget
{
	OneShotWidget(OneShot *module)
	{
		setModule(module);

		PanelHelper panelHelper(this);
		panelHelper.loadPanel(
			asset::plugin(pluginInstance, "res/modules/one_shot/one_shot_panel.svg"),
			asset::plugin(pluginInstance, "res/modules/one_shot/one_shot_panel-dark.svg")
		);

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// TODO: Add knobs, buttons, ports, and displays once panel SVG is designed.
		// Use panelHelper.findNamed() to position components from SVG named elements.
		//
		// Example:
		// addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("wav_selector_knob"), module, OneShot::WAV_SELECTOR_PARAM));
		// addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trigger_input"), module, OneShot::TRIGGER_INPUT));
		// addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, OneShot::OUTPUT_L));
	}

	void appendContextMenu(Menu *menu) override
	{
		OneShot *module = dynamic_cast<OneShot *>(this->module);
		assert(module);

		menu->addChild(new MenuEntry);

		// Load folder menu item
		MenuItemLoadBankOneShot *menu_item_load_bank = new MenuItemLoadBankOneShot();
		menu_item_load_bank->text = "Select Directory Containing WAV Files";
		menu_item_load_bank->module = module;
		menu->addChild(menu_item_load_bank);
	}
};
