struct AutobreakWidget : VoxglitchSamplerModuleWidget
{
	AutobreakWidget(Autobreak *module)
	{
		setModule(module);

		// Load and apply theme
		theme.load("autobreak");
		applyTheme();

		// =================== PLACE COMPONENTS ====================================

		addParam(createParamCentered<VoxglitchLargeKnob>(themePos("WAV_KNOB"), module, Autobreak::WAV_KNOB));
		addParam(createParamCentered<VoxglitchAttenuator>(themePos("WAV_ATTN_KNOB"), module, Autobreak::WAV_ATTN_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("WAV_INPUT"), module, Autobreak::WAV_INPUT));

		addInput(createInputCentered<VoxglitchInputPort>(themePos("CLOCK_INPUT"), module, Autobreak::CLOCK_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, Autobreak::RESET_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("SEQUENCE_INPUT"), module, Autobreak::SEQUENCE_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("RATCHET_INPUT"), module, Autobreak::RATCHET_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("REVERSE_INPUT"), module, Autobreak::REVERSE_INPUT));

		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_LEFT"), module, Autobreak::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_RIGHT"), module, Autobreak::AUDIO_OUTPUT_RIGHT));
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
			// AutobreakLoadSample *menu_item_load_sample = new AutobreakLoadSample;
			SampleLoaderMenuItem<Autobreak> *menu_item_load_sample = new SampleLoaderMenuItem<Autobreak>;

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
