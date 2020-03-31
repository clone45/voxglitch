struct WavBankWidget : ModuleWidget
{
	WavBankWidget(WavBank* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/wav_bank_front_panel.svg")));

		// Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

		// Input and label for the trigger input (which is labeled "CLK" on the front panel)
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 25.535)), module, WavBank::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 46)), module, WavBank::WAV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.185, 114.893)), module, WavBank::PITCH_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(13.185, 60)), module, WavBank::WAV_ATTN_KNOB));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(13.185, 75)), module, WavBank::WAV_KNOB));
		addParam(createParamCentered<CKSS>(mm2px(Vec(13.185, 97)), module, WavBank::LOOP_SWITCH));

		WavBankReadout *readout = new WavBankReadout();
		readout->box.pos = mm2px(Vec(34.236, 82));
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);

		// WAV output
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 104)), module, WavBank::WAV_LEFT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 114.9)), module, WavBank::WAV_RIGHT_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		WavBank *module = dynamic_cast<WavBank*>(this->module);
		assert(module);

		// For spacing only
		menu->addChild(new MenuEntry);

		// Add the "Select Directory Containing WAV Files" menu item
		MenuItemLoadBank *menu_item_load_bank = new MenuItemLoadBank();
		menu_item_load_bank->text = "Select Directory Containing WAV Files";
		menu_item_load_bank->wav_bank_module = module;
		menu->addChild(menu_item_load_bank);
	}

};
