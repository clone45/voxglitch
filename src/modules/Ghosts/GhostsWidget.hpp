struct GhostsWidget : VoxglitchSamplerModuleWidget
{
	Ghosts *module;

	GhostsWidget(Ghosts *module)
	{
		this->module = module;
		setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/ghosts/ghosts_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/ghosts/ghosts_panel-dark.svg")
        );

		// =================== PLACE COMPONENTS ====================================

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Purge
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("purge_input"), module, Ghosts::PURGE_TRIGGER_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("purge_button"), module, Ghosts::PURGE_BUTTON_PARAM, Ghosts::PURGE_LIGHT));

		// Jitter
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("jitter_input"), module, Ghosts::JITTER_CV_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("jitter_button"), module, Ghosts::JITTER_SWITCH, Ghosts::JITTER_LIGHT));

		// Position
		addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("position_knob"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("position_input"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_INPUT));
		addParam(createParamCentered<Trimpot>(panelHelper.findNamed("position_attn_knob"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

		// Pitch
		addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pitch_knob"), module, Ghosts::PITCH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pitch_input"), module, Ghosts::PITCH_INPUT));

		// Length
		addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("length_knob"), module, Ghosts::GHOST_PLAYBACK_LENGTH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("length_input"), module, Ghosts::GHOST_PLAYBACK_LENGTH_INPUT));
		addParam(createParamCentered<Trimpot>(panelHelper.findNamed("length_attn_knob"), module, Ghosts::GHOST_PLAYBACK_LENGTH_ATTN_KNOB));

		// Graveyard Capacity
		addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("quota_knob"), module, Ghosts::GRAVEYARD_CAPACITY_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("quota_input"), module, Ghosts::GRAVEYARD_CAPACITY_INPUT));
		addParam(createParamCentered<Trimpot>(panelHelper.findNamed("quota_attn_knob"), module, Ghosts::GRAVEYARD_CAPACITY_ATTN_KNOB));

		// Spawn rate
		addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("spawn_knob"), module, Ghosts::GHOST_SPAWN_RATE_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("spawn_input"), module, Ghosts::GHOST_SPAWN_RATE_INPUT));
		addParam(createParamCentered<Trimpot>(panelHelper.findNamed("spawn_attn_knob"), module, Ghosts::GHOST_SPAWN_RATE_ATTN_KNOB));

		// Trim
		addParam(createParamCentered<Trimpot>(panelHelper.findNamed("trim_knob"), module, Ghosts::TRIM_KNOB));

		// WAV output
		addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, Ghosts::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, Ghosts::AUDIO_OUTPUT_RIGHT));

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

		// Add Remove Sample menu item
		menu->addChild(createMenuItem("Remove Sample", "", [=]() {
			module->sample.unload();
			module->loaded_filename = "[ EMPTY ]";
			module->graveyard.markAllForRemoval();  // Clear any playing ghosts
		}));
	}
};
