struct AutobreakStudioWidget : VoxglitchSamplerModuleWidget
{
	AutobreakStudioWidget(AutobreakStudio *module)
	{
		setModule(module);

		// Load and apply theme
		theme.load("autobreak_studio");
		applyTheme();

		// =================== PLACE COMPONENTS ====================================

		addParam(createParamCentered<VoxglitchLargeKnob>(themePos("WAV_KNOB"), module, AutobreakStudio::WAV_KNOB));
		addParam(createParamCentered<VoxglitchAttenuator>(themePos("WAV_ATTN_KNOB"), module, AutobreakStudio::WAV_ATTN_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("WAV_INPUT"), module, AutobreakStudio::WAV_INPUT));

		addInput(createInputCentered<VoxglitchInputPort>(themePos("CLOCK_INPUT"), module, AutobreakStudio::CLOCK_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, AutobreakStudio::RESET_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("SEQUENCE_INPUT"), module, AutobreakStudio::SEQUENCE_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("RATCHET_INPUT"), module, AutobreakStudio::RATCHET_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("REVERSE_INPUT"), module, AutobreakStudio::REVERSE_INPUT));

		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_LEFT"), module, AutobreakStudio::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_RIGHT"), module, AutobreakStudio::AUDIO_OUTPUT_RIGHT));

		for(unsigned int t=0; t<NUMBER_OF_STEPS; t++)
		{
			addParam(createParamCentered<squareToggle>(themePos("GATE_TOGGLE_" + std::to_string(t)), module, AutobreakStudio::GATE_TOGGLE_BUTTONS + t));
			addParam(createParamCentered<squareToggle>(themePos("RATCHET_TOGGLE_" + std::to_string(t)), module, AutobreakStudio::RATCHET_TOGGLE_BUTTONS + t));

		}

	}

	void appendContextMenu(Menu *menu) override
	{
		AutobreakStudio *module = dynamic_cast<AutobreakStudio *>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Samples"));

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		std::string menu_text = "#";

		for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
		{
			AutobreakStudioLoadSample *menu_item_load_sample = new AutobreakStudioLoadSample;
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
