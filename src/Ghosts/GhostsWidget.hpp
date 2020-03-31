struct GhostsWidget : ModuleWidget
{
	GhostsWidget(Ghosts* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ghosts_front_panel.svg")));

		// Purge
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, 25.974)), module, Ghosts::PURGE_TRIGGER_INPUT));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(75.595, 25.974)), module, Ghosts::PURGE_BUTTON_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(75.595, 25.974)), module, Ghosts::PURGE_LIGHT));

		// Jitter
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, 45.713)), module, Ghosts::JITTER_CV_INPUT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(75.595, 45.713)), module, Ghosts::JITTER_SWITCH));

		//
		// Main Left-side Knobs
		//

		float y_offset = 1.8;
		float x_offset = -1.8;

		// Position
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 28.526 - y_offset)), module, Ghosts::SAMPLE_PLAYBACK_POSITION_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 28.526 - y_offset)), module, Ghosts::SAMPLE_PLAYBACK_POSITION_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 28.526 - y_offset)), module, Ghosts::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

		// Pitch
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 50.489 - y_offset)), module, Ghosts::PITCH_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 50.489 - y_offset)), module, Ghosts::PITCH_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 50.489 - y_offset)), module, Ghosts::PITCH_ATTN_KNOB));

		// Length
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 72.452 - y_offset)), module, Ghosts::GHOST_PLAYBACK_LENGTH_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 72.452 - y_offset)), module, Ghosts::GHOST_PLAYBACK_LENGTH_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 72.452 - y_offset)), module, Ghosts::GHOST_PLAYBACK_LENGTH_ATTN_KNOB));

		// Graveyard Capacity
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 94.416 - y_offset)), module, Ghosts::GRAVEYARD_CAPACITY_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 94.416 - y_offset)), module, Ghosts::GRAVEYARD_CAPACITY_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 94.416 - y_offset)), module, Ghosts::GRAVEYARD_CAPACITY_ATTN_KNOB));

		// Spawn rate
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 116.634 - y_offset)), module, Ghosts::GHOST_SPAWN_RATE_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 116.634 - y_offset)), module, Ghosts::GHOST_SPAWN_RATE_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 116.634 - y_offset)), module, Ghosts::GHOST_SPAWN_RATE_ATTN_KNOB));

		// Trim
		addParam(createParamCentered<Trimpot>(mm2px(Vec(75.470, 103.043)), module, Ghosts::TRIM_KNOB));

		// WAV output

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.746, 114.702)), module, Ghosts::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(75.470, 114.702)), module, Ghosts::AUDIO_OUTPUT_RIGHT));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.236, 124.893)), module, Ghosts::DEBUG_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override
	{
		Ghosts *module = dynamic_cast<Ghosts*>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Sample"));

		GhostsLoadSample *menu_item_load_sample = new GhostsLoadSample();
		menu_item_load_sample->text = module->loaded_filename;
		menu_item_load_sample->module = module;
		menu->addChild(menu_item_load_sample);
	}

};
