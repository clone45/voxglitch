struct AutobreakWidget : VoxglitchSamplerModuleWidget
{
	AutobreakWidget(Autobreak *module)
	{
		setModule(module);

//		setPanel(createPanel(
//			asset::plugin(pluginInstance, "res/autobreak/autobreak_panel.svg"),
//			asset::plugin(pluginInstance, "res/autobreak/autobreak_panel-dark.svg")
//		));

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
			asset::plugin(pluginInstance, "res/autobreak/autobreak_panel.svg"),
			asset::plugin(pluginInstance, "res/autobreak/autobreak_panel-dark.svg")
		);

		// =================== PLACE COMPONENTS ====================================

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundHugeBlackKnob>(panelHelper.findNamed("wav_knob"), module, Autobreak::WAV_KNOB));
		addParam(createParamCentered<Trimpot>(panelHelper.findNamed("wav_attn_knob"), module, Autobreak::WAV_ATTN_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("wav_input"), module, Autobreak::WAV_INPUT));

		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("clock_input"), module, Autobreak::CLOCK_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, Autobreak::RESET_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("sequence_input"), module, Autobreak::SEQUENCE_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("ratchet_input"), module, Autobreak::RATCHET_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reverse_input"), module, Autobreak::REVERSE_INPUT));

		addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("audio_output_left"), module, Autobreak::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("audio_output_right"), module, Autobreak::AUDIO_OUTPUT_RIGHT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Autobreak *module = dynamic_cast<Autobreak *>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Samples"));

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		std::string menu_text = "#";

		for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
		{
			AutobreakLoadSample *menu_item_load_sample = new AutobreakLoadSample;
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = std::to_string(i + 1) + ": " + module->loaded_filenames[i];
			menu_item_load_sample->module = module;
			menu->addChild(menu_item_load_sample);
		}

		menu->addChild(new MenuEntry); // For spacing only
		SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
		sample_interpolation_menu_item->module = module;
		menu->addChild(sample_interpolation_menu_item);
	}
};
