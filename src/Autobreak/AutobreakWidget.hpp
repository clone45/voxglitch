struct AutobreakWidget : ModuleWidget
{
	AutobreakWidget(Autobreak* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/autobreak_front_panel.svg")));

		// Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

		float y_offset = -10;

		// Inputs for WAV selection
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10, 38 + y_offset)), module, Autobreak::WAV_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10, 52 + y_offset)), module, Autobreak::WAV_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 63.5 + y_offset)), module, Autobreak::WAV_INPUT));

		//
		// Controls to the right of the sequencer

		// Reset
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.121, 66.984)), module, Autobreak::RESET_INPUT));

        // Sequence
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.121, 47.008)), module, Autobreak::SEQUENCE_INPUT));

        // Clock
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.121, 27.032)), module, Autobreak::CLOCK_INPUT));

        // Audio outputs
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.121, 104)), module, Autobreak::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.121, 114.9)), module, Autobreak::AUDIO_OUTPUT_RIGHT));
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
	}
};
