struct GhostsWidget : VoxglitchSamplerModuleWidget
{
	GhostsWidget(Ghosts* module)
	{
		setModule(module);

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
		addParam(createParamCentered<VoxglitchEpicKnob>(themePos("SAMPLE_PLAYBACK_POSITION_KNOB"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("SAMPLE_PLAYBACK_POSITION_INPUT"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_INPUT));
		addParam(createParamCentered<VoxglitchAttenuator>(themePos("SAMPLE_PLAYBACK_POSITION_ATTN_KNOB"), module, Ghosts::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

		// Pitch
		addParam(createParamCentered<VoxglitchMediumBlackKnob>(themePos("PITCH_KNOB"), module, Ghosts::PITCH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("PITCH_INPUT"), module, Ghosts::PITCH_INPUT));
		// addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 50.489 - y_offset)), module, Ghosts::PITCH_ATTN_KNOB));

		// Length
		addParam(createParamCentered<VoxglitchMediumBlackKnob>(themePos("GHOST_PLAYBACK_LENGTH_KNOB"), module, Ghosts::GHOST_PLAYBACK_LENGTH_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("GHOST_PLAYBACK_LENGTH_INPUT"), module, Ghosts::GHOST_PLAYBACK_LENGTH_INPUT));
		addParam(createParamCentered<VoxglitchAttenuator>(themePos("GHOST_PLAYBACK_LENGTH_ATTN_KNOB"), module, Ghosts::GHOST_PLAYBACK_LENGTH_ATTN_KNOB));

		// Graveyard Capacity
		addParam(createParamCentered<VoxglitchMediumBlackKnob>(themePos("GRAVEYARD_CAPACITY_KNOB"), module, Ghosts::GRAVEYARD_CAPACITY_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("GRAVEYARD_CAPACITY_INPUT"), module, Ghosts::GRAVEYARD_CAPACITY_INPUT));
		addParam(createParamCentered<VoxglitchAttenuator>(themePos("GRAVEYARD_CAPACITY_ATTN_KNOB"), module, Ghosts::GRAVEYARD_CAPACITY_ATTN_KNOB));

		// Spawn rate
		addParam(createParamCentered<VoxglitchMediumBlackKnob>(themePos("GHOST_SPAWN_RATE_KNOB"), module, Ghosts::GHOST_SPAWN_RATE_KNOB));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("GHOST_SPAWN_RATE_INPUT"), module, Ghosts::GHOST_SPAWN_RATE_INPUT));
		addParam(createParamCentered<VoxglitchAttenuator>(themePos("GHOST_SPAWN_RATE_ATTN_KNOB"), module, Ghosts::GHOST_SPAWN_RATE_ATTN_KNOB));

		// Trim
		addParam(createParamCentered<VoxglitchAttenuator>(themePos("TRIM_KNOB"), module, Ghosts::TRIM_KNOB));

		// WAV output
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_LEFT"), module, Ghosts::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_RIGHT"), module, Ghosts::AUDIO_OUTPUT_RIGHT));
	}

  struct ModeValueItem : MenuItem {
    Ghosts *module;
    int option = 0;

    void onAction(const event::Action &e) override {
      module->mode = this->option;
    }
  };

  struct ModeMenuItem : MenuItem {
    Ghosts *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      // trigger location == 0 (bottom) is default
      ModeValueItem *mode_0 = createMenuItem<ModeValueItem>("Mode 0", CHECKMARK(module->mode == 0));
      mode_0->module = module;
      mode_0->option = 0;
      menu->addChild(mode_0);

      ModeValueItem *mode_1 = createMenuItem<ModeValueItem>("Mode 1", CHECKMARK(module->mode == 1));
      mode_1->module = module;
      mode_1->option = 1;
      menu->addChild(mode_1);

      ModeValueItem *mode_2 = createMenuItem<ModeValueItem>("Mode 2", CHECKMARK(module->mode == 2));
      mode_2->module = module;
      mode_2->option = 2;
      menu->addChild(mode_2);

      ModeValueItem *mode_3 = createMenuItem<ModeValueItem>("Mode 3", CHECKMARK(module->mode == 3));
      mode_3->module = module;
      mode_3->option = 3;
      menu->addChild(mode_3);

      return menu;
    }
  };

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

    ModeMenuItem *mode_menu_item = createMenuItem<ModeMenuItem>("Mode", RIGHT_ARROW);
    mode_menu_item->module = module;
    menu->addChild(mode_menu_item);
	}

};
