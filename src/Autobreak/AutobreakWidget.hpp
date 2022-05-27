struct AutobreakWidget : VoxglitchSamplerModuleWidget
{
	AutobreakWidget(Autobreak* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/autobreak_front_panel.svg")));

		// Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

    float column_1 = 10; // left column
    float column_2 = 30.121;

    // These values match Ghosts
    float row_1 = 26.726; // Top row
    float row_2 = 48.689;
    float row_3 = 70.652;
    float row_35 = 85;
    float row_4 = 100;

    // Clock Input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_1, row_1)), module, Autobreak::CLOCK_INPUT));

    // Reset
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_2, row_1)), module, Autobreak::RESET_INPUT));

    // Sequence
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_1, row_2)), module, Autobreak::SEQUENCE_INPUT));

    // Ratchet input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_2, row_2)), module, Autobreak::RATCHET_INPUT));

    // Ratchet input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_2, row_3)), module, Autobreak::REVERSE_INPUT));

		// Inputs for SAMPLE selection
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(column_1, row_4)), module, Autobreak::WAV_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(column_1, row_35)), module, Autobreak::WAV_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(column_1, row_3)), module, Autobreak::WAV_INPUT));

    // Audio outputs
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2, 104)), module, Autobreak::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2, 114.9)), module, Autobreak::AUDIO_OUTPUT_RIGHT));
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
