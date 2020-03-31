struct GoblinsWidget : ModuleWidget
{
	GoblinsWidget(Goblins* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/goblins_front_panel.svg")));

		// Purge
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 45)), module, Goblins::PURGE_TRIGGER_INPUT));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(17, 52)), module, Goblins::PURGE_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(17, 52)), module, Goblins::PURGE_LIGHT));

		// Position
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(35, 33)), module, Goblins::SAMPLE_PLAYBACK_POSITION_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(54, 33)), module, Goblins::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(67, 33)), module, Goblins::SAMPLE_PLAYBACK_POSITION_INPUT));

		// Length
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(10, 90)), module, Goblins::PLAYBACK_LENGTH_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10, 104)), module, Goblins::PLAYBACK_LENGTH_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 115.5)), module, Goblins::PLAYBACK_LENGTH_INPUT));

		// Rate
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(30, 90)), module, Goblins::SPAWN_RATE_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30, 104)), module, Goblins::SPAWN_RATE_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 115.5)), module, Goblins::SPAWN_RATE_INPUT));

		// Quantity (Countryside Capacity)
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(50, 90)), module, Goblins::COUNTRYSIDE_CAPACITY_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(50, 104)), module, Goblins::COUNTRYSIDE_CAPACITY_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50, 115.5)), module, Goblins::COUNTRYSIDE_CAPACITY_INPUT));

		// Pitch
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(70, 90)), module, Goblins::PITCH_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(70, 104)), module, Goblins::PITCH_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(70, 115.5)), module, Goblins::PITCH_INPUT));

		// Sample
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(90, 90)), module, Goblins::SAMPLE_SELECT_KNOB));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(90, 104)), module, Goblins::SAMPLE_SELECT_ATTN_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(90, 115.5)), module, Goblins::SAMPLE_SELECT_INPUT));

		// Trim / Wav Output
		// addParam(createParamCentered<Trimpot>(mm2px(Vec(88.6, 37)), module, Goblins::TRIM_KNOB));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(91.874, 44.611)), module, Goblins::WAV_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(91.874, 34.052)), module, Goblins::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(91.874, 44.611)), module, Goblins::AUDIO_OUTPUT_RIGHT));

		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 124.893)), module, Ghosts::DEBUG_OUTPUT));

		GoblinsSampleReadout *readout = new GoblinsSampleReadout();
		readout->box.pos = mm2px(Vec(8, 64.3));
		readout->box.size = Vec(200, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);
	}

	void appendContextMenu(Menu *menu) override
	{
		Goblins *module = dynamic_cast<Goblins*>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Samples"));

		//
		// Add the sample slots to the right-click context menu
		//

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			GoblinsLoadSample *menu_item_load_sample = new GoblinsLoadSample();
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = std::to_string(i+1) + ": " + module->loaded_filenames[i];
			menu_item_load_sample->module = module;
			menu->addChild(menu_item_load_sample);
		}
	}
};
