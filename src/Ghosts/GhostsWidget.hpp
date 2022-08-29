struct GhostsModesKnob : RoundBlackKnob {
	GhostsModesKnob() {
		minAngle = -0.75*M_PI;
		maxAngle = 0*M_PI;
	}
};

struct GhostsWidget : VoxglitchSamplerModuleWidget
{
	GhostsWidget(Ghosts* module)
	{
		setModule(module);

		// setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ghosts_front_panel.svg")));

    // Load and apply theme
    theme.load("ghosts");
    applyTheme();

    // =================== PLACE COMPONENTS ====================================

    if(theme.showScrews())
    {
      addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, 0)));
      addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
      addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
      addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

		// Purge
		addInput(createInputCentered<VoxglitchInputPort>(themePos("PURGE_TRIGGER_INPUT"), module, Ghosts::PURGE_TRIGGER_INPUT));
		addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("PURGE_BUTTON_PARAM"), module, Ghosts::PURGE_BUTTON_PARAM));
		// addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(75.595, 25.974)), module, Ghosts::PURGE_LIGHT));

		// Jitter
		addInput(createInputCentered<VoxglitchInputPort>(themePos("JITTER_CV_INPUT"), module, Ghosts::JITTER_CV_INPUT));
		addParam(createParamCentered<VoxglitchRoundLampSwitch>(themePos("JITTER_SWITCH"), module, Ghosts::JITTER_SWITCH));

    // Modes
    // addParam(createParamCentered<GhostsModesKnob>(mm2px(Vec(62.366, 65)), module, Ghosts::MODES_KNOB));

		// Position
		addParam(createParamCentered<RoundBlackKnob>(themePos("SAMPLE_PLAYBACK_POSITION_KNOB"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("SAMPLE_PLAYBACK_POSITION_INPUT"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_INPUT));
		addParam(createParamCentered<Trimpot>(themePos("SAMPLE_PLAYBACK_POSITION_ATTN_KNOB"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

		// Pitch
		addParam(createParamCentered<RoundBlackKnob>(themePos("PITCH_KNOB"), module, Ghosts::PITCH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("PITCH_INPUT"), module, Ghosts::PITCH_INPUT));
		// addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 50.489 - y_offset)), module, Ghosts::PITCH_ATTN_KNOB));

		// Length
		addParam(createParamCentered<RoundBlackKnob>(themePos("GHOST_PLAYBACK_LENGTH_KNOB"), module, Ghosts::GHOST_PLAYBACK_LENGTH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("GHOST_PLAYBACK_LENGTH_INPUT"), module, Ghosts::GHOST_PLAYBACK_LENGTH_INPUT));
		addParam(createParamCentered<Trimpot>(themePos("GHOST_PLAYBACK_LENGTH_ATTN_KNOB"), module, Ghosts::GHOST_PLAYBACK_LENGTH_ATTN_KNOB));

		// Graveyard Capacity
		addParam(createParamCentered<RoundBlackKnob>(themePos("GRAVEYARD_CAPACITY_KNOB"), module, Ghosts::GRAVEYARD_CAPACITY_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("GRAVEYARD_CAPACITY_INPUT"), module, Ghosts::GRAVEYARD_CAPACITY_INPUT));
		addParam(createParamCentered<Trimpot>(themePos("GRAVEYARD_CAPACITY_ATTN_KNOB"), module, Ghosts::GRAVEYARD_CAPACITY_ATTN_KNOB));

		// Spawn rate
		addParam(createParamCentered<RoundBlackKnob>(themePos("GHOST_SPAWN_RATE_KNOB"), module, Ghosts::GHOST_SPAWN_RATE_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("GHOST_SPAWN_RATE_INPUT"), module, Ghosts::GHOST_SPAWN_RATE_INPUT));
		addParam(createParamCentered<Trimpot>(themePos("GHOST_SPAWN_RATE_ATTN_KNOB"), module, Ghosts::GHOST_SPAWN_RATE_ATTN_KNOB));

		// Trim
		addParam(createParamCentered<Trimpot>(themePos("TRIM_KNOB"), module, Ghosts::TRIM_KNOB));

		// WAV output

		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_LEFT"), module, Ghosts::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_RIGHT"), module, Ghosts::AUDIO_OUTPUT_RIGHT));
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
