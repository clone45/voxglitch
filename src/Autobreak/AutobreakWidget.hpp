struct AutobreakWidget : VoxglitchSamplerModuleWidget
{
	AutobreakWidget(Autobreak* module)
	{
		setModule(module);

    // Load and apply theme
    theme.load("autobreak");
    applyTheme();

    // =================== PLACE COMPONENTS ====================================

    addParam(createParamCentered<VoxglitchLargeKnob>(Vec(60.053509,83.487495), module, Autobreak::WAV_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(32.749992,136.200012), module, Autobreak::WAV_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(86.650009,136.050018), module, Autobreak::WAV_INPUT));

    addInput(createInputCentered<VoxglitchInputPort>(Vec(33.149986,198.750000), module, Autobreak::CLOCK_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(86.950012,198.750000), module, Autobreak::RESET_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(33.199993,250.800018), module, Autobreak::SEQUENCE_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(86.850021,250.900024), module, Autobreak::RATCHET_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(32.900002,303.149841), module, Autobreak::REVERSE_INPUT));

    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(92.500015,303.249878), module, Autobreak::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(92.550034,354.949951), module, Autobreak::AUDIO_OUTPUT_RIGHT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Autobreak *module = dynamic_cast<Autobreak*>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Samples"));

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		std::string menu_text = "#";

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			AutobreakLoadSample *menu_item_load_sample = new AutobreakLoadSample;
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = std::to_string(i+1) + ": " + module->loaded_filenames[i];
			menu_item_load_sample->module = module;
			menu->addChild(menu_item_load_sample);
		}

    menu->addChild(new MenuEntry); // For spacing only
    SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
    sample_interpolation_menu_item->module = module;
    menu->addChild(sample_interpolation_menu_item);
	}
};
