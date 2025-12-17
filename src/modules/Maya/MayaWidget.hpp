#pragma once

struct MayaWidget : VoxglitchSamplerModuleWidget
{
	MayaWidget(Maya *module)
	{
		setModule(module);

		PanelHelper panelHelper(this);
		panelHelper.loadPanel(
			asset::plugin(pluginInstance, "res/modules/maya/maya_panel.svg"),
			asset::plugin(pluginInstance, "res/modules/maya/maya_panel-dark.svg")
		);

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Word selection knob
		addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("wav_knob"), module, Maya::WORD_KNOB));

		// Word selection attenuator
		addParam(createParamCentered<Trimpot>(panelHelper.findNamed("wav_attn_knob"), module, Maya::WORD_ATTN_KNOB));

		// Word CV input
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("wav_input"), module, Maya::WORD_INPUT));

		// Bank selection knob
		addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("bank_knob"), module, Maya::BANK_KNOB));

		// Bank selection attenuator
		addParam(createParamCentered<Trimpot>(panelHelper.findNamed("bank_attn_knob"), module, Maya::BANK_ATTN_KNOB));

		// Bank CV input
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("bank_input"), module, Maya::BANK_INPUT));

		// Pitch input (1V/oct)
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pitch_input"), module, Maya::PITCH_INPUT));

		// Trigger input
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input"), module, Maya::TRIG_INPUT));

		// Audio outputs
		addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, Maya::AUDIO_LEFT_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, Maya::AUDIO_RIGHT_OUTPUT));

		// Word display - centered on lcd_display element
		MayaWordDisplay *word_display = new MayaWordDisplay();
		word_display->box.size = Vec(110, 30);
		Vec lcd_center = panelHelper.findNamed("lcd_display");
		word_display->box.pos = Vec(lcd_center.x - word_display->box.size.x / 2, lcd_center.y - word_display->box.size.y / 2);
		word_display->module = module;
		addChild(word_display);
	}

	void appendContextMenu(Menu *menu) override
	{
		Maya *module = dynamic_cast<Maya *>(this->module);
		assert(module);

		menu->addChild(new MenuEntry);

		// Add the "Select Directory Containing Word Files" menu item
		MenuItemLoadWords *menu_item_load_words = new MenuItemLoadWords();
		menu_item_load_words->text = "Select Directory Containing Word Files";
		menu_item_load_words->module = module;
		menu->addChild(menu_item_load_words);

		// Sample interpolation settings
		menu->addChild(new MenuEntry);
		SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
		sample_interpolation_menu_item->module = module;
		menu->addChild(sample_interpolation_menu_item);

		// Pitch CV mode selection
		menu->addChild(createIndexSubmenuItem("Pitch CV Range",
			{
				"Bipolar: -5V to +5V",
				"Unipolar: 0V to 10V"
			},
			[=]() {
				return module->pitch_cv_mode;
			},
			[=](int mode) {
				module->pitch_cv_mode = mode;
			}
		));
	}
};
