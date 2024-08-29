struct GhostsWidget : VoxglitchSamplerModuleWidget
{
	Ghosts *module;

	GhostsWidget(Ghosts *module)
	{
		this->module = module;
		setModule(module);

		// Load and apply theme
		// theme.load("ghosts");
		// applyTheme();

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
		addInput(createInputCentered<VoxglitchInputPort>(Vec(33.96638, 342.80079), module, Ghosts::PURGE_TRIGGER_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(71.59456, 329.94108), module, Ghosts::PURGE_BUTTON_PARAM, Ghosts::PURGE_LIGHT));

		// Jitter
		addInput(createInputCentered<VoxglitchInputPort>(Vec(108.46513, 342.80079), module, Ghosts::JITTER_CV_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(71.59456, 357.40460), module, Ghosts::JITTER_SWITCH, Ghosts::JITTER_LIGHT));

		// Position
		addParam(createParamCentered<RoundBlackKnob>(Vec(38.27531, 130.0), module, Ghosts::SAMPLE_PLAYBACK_POSITION_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(38.27531, 50.58442), module, Ghosts::SAMPLE_PLAYBACK_POSITION_INPUT));
		addParam(createParamCentered<Trimpot>(Vec(38.27531, 84.46070), module, Ghosts::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

		// Pitch
		addParam(createParamCentered<RoundLargeBlackKnob>(Vec(38.27531, 206.17370), module, Ghosts::PITCH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(38.27531, 284.59863), module, Ghosts::PITCH_INPUT));

		// Length
		addParam(createParamCentered<RoundLargeBlackKnob>(Vec(103.48146, 206.17370), module, Ghosts::GHOST_PLAYBACK_LENGTH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(103.48146, 284.59863), module, Ghosts::GHOST_PLAYBACK_LENGTH_INPUT));
		addParam(createParamCentered<Trimpot>(Vec(103.48146, 251.35435), module, Ghosts::GHOST_PLAYBACK_LENGTH_ATTN_KNOB));

		// Graveyard Capacity
		addParam(createParamCentered<RoundLargeBlackKnob>(Vec(168.78623, 206.17370), module, Ghosts::GRAVEYARD_CAPACITY_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(168.78623, 284.59863), module, Ghosts::GRAVEYARD_CAPACITY_INPUT));
		addParam(createParamCentered<Trimpot>(Vec(168.78623, 251.35435), module, Ghosts::GRAVEYARD_CAPACITY_ATTN_KNOB));

		// Spawn rate
		addParam(createParamCentered<RoundLargeBlackKnob>(Vec(232.01941, 206.17370), module, Ghosts::GHOST_SPAWN_RATE_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(232.01941, 284.59863), module, Ghosts::GHOST_SPAWN_RATE_INPUT));
		addParam(createParamCentered<Trimpot>(Vec(232.01941, 251.35435), module, Ghosts::GHOST_SPAWN_RATE_ATTN_KNOB));

		// Trim
		addParam(createParamCentered<Trimpot>(Vec(162.78623, 342.80079), module, Ghosts::TRIM_KNOB));

		// WAV output
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(207.14441, 342.80079), module, Ghosts::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(243.19423, 342.80079), module, Ghosts::AUDIO_OUTPUT_RIGHT));

		// Waveform visualizer
		if(module)
		{
			WaveformWidget *waveform_widget = new WaveformWidget(WAVEFORM_WIDGET_WIDTH, WAVEFORM_WIDGET_HEIGHT, &this->module->waveform_model);
			waveform_widget->box.pos = Vec(70.0, 45.0);
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
