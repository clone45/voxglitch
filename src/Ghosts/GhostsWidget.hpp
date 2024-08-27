struct GhostsWidget : VoxglitchSamplerModuleWidget
{
	Ghosts *module;

	GhostsWidget(Ghosts *module)
	{
		this->module = module;
		setModule(module);

		// Load and apply theme
		theme.load("ghosts");
		applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/ghosts/ghosts_panel.svg"),
            asset::plugin(pluginInstance, "res/ghosts/ghosts_panel-dark.svg")
        ));

		// =================== PLACE COMPONENTS ====================================

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Purge
		addInput(createInputCentered<VoxglitchInputPort>(themePos("PURGE_TRIGGER_INPUT"), module, Ghosts::PURGE_TRIGGER_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("PURGE_BUTTON_PARAM"), module, Ghosts::PURGE_BUTTON_PARAM, Ghosts::PURGE_LIGHT));
		// addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(75.595, 25.974)), module, Ghosts::PURGE_LIGHT));

		// Jitter
		addInput(createInputCentered<VoxglitchInputPort>(themePos("JITTER_CV_INPUT"), module, Ghosts::JITTER_CV_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("JITTER_SWITCH"), module, Ghosts::JITTER_SWITCH, Ghosts::JITTER_LIGHT));

		// Modes
		// addParam(createParamCentered<GhostsModesKnob>(mm2px(Vec(62.366, 65)), module, Ghosts::MODES_KNOB));

		// Position
		addParam(createParamCentered<RoundBlackKnob>(themePos("SAMPLE_PLAYBACK_POSITION_KNOB"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("SAMPLE_PLAYBACK_POSITION_INPUT"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_INPUT));
		addParam(createParamCentered<Trimpot>(themePos("SAMPLE_PLAYBACK_POSITION_ATTN_KNOB"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

		// Pitch
		addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PITCH_KNOB"), module, Ghosts::PITCH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("PITCH_INPUT"), module, Ghosts::PITCH_INPUT));
		// addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 50.489 - y_offset)), module, Ghosts::PITCH_ATTN_KNOB));

		// Length
		addParam(createParamCentered<RoundLargeBlackKnob>(themePos("GHOST_PLAYBACK_LENGTH_KNOB"), module, Ghosts::GHOST_PLAYBACK_LENGTH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("GHOST_PLAYBACK_LENGTH_INPUT"), module, Ghosts::GHOST_PLAYBACK_LENGTH_INPUT));
		addParam(createParamCentered<Trimpot>(themePos("GHOST_PLAYBACK_LENGTH_ATTN_KNOB"), module, Ghosts::GHOST_PLAYBACK_LENGTH_ATTN_KNOB));

		// Graveyard Capacity
		addParam(createParamCentered<RoundLargeBlackKnob>(themePos("GRAVEYARD_CAPACITY_KNOB"), module, Ghosts::GRAVEYARD_CAPACITY_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("GRAVEYARD_CAPACITY_INPUT"), module, Ghosts::GRAVEYARD_CAPACITY_INPUT));
		addParam(createParamCentered<Trimpot>(themePos("GRAVEYARD_CAPACITY_ATTN_KNOB"), module, Ghosts::GRAVEYARD_CAPACITY_ATTN_KNOB));

		// Spawn rate
		addParam(createParamCentered<RoundLargeBlackKnob>(themePos("GHOST_SPAWN_RATE_KNOB"), module, Ghosts::GHOST_SPAWN_RATE_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("GHOST_SPAWN_RATE_INPUT"), module, Ghosts::GHOST_SPAWN_RATE_INPUT));
		addParam(createParamCentered<Trimpot>(themePos("GHOST_SPAWN_RATE_ATTN_KNOB"), module, Ghosts::GHOST_SPAWN_RATE_ATTN_KNOB));

		// Trim
		addParam(createParamCentered<Trimpot>(themePos("TRIM_KNOB"), module, Ghosts::TRIM_KNOB));

		// WAV output
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_LEFT"), module, Ghosts::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_RIGHT"), module, Ghosts::AUDIO_OUTPUT_RIGHT));

		// Waveform visualizer
		if(module)
		{
			WaveformWidget *waveform_widget = new WaveformWidget(WAVEFORM_WIDGET_WIDTH, WAVEFORM_WIDGET_HEIGHT, &this->module->waveform_model);
			waveform_widget->box.pos = themePos("WAVEFORM_DISPLAY");
			waveform_widget->hide();
			addChild(waveform_widget);
		}
	}

	void appendContextMenu(Menu *menu) override
	{
		Ghosts *module = dynamic_cast<Ghosts *>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Sample"));

		GhostsLoadSample *menu_item_load_sample = new GhostsLoadSample();
		menu_item_load_sample->text = module->loaded_filename;
		menu_item_load_sample->module = module;
		menu->addChild(menu_item_load_sample);
	}
};
