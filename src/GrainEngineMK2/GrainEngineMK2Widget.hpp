struct GrainEngineMK2Widget : VoxglitchModuleWidget
{
  GrainEngineMK2Widget(GrainEngineMK2* module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("grain_engine_mk2");
    applyTheme();

    // Add rack screws
    if(theme.showScrews())
    {
  		addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, 0)));
  		addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  		addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  		addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    //
    // Control inputs in the center area of the module
    //
    addParam(createParamCentered<VoxglitchLargeKnob>(themePos("POSITION_KNOB"), module, GrainEngineMK2::POSITION_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("POSITION_ATTN_KNOB"), module, GrainEngineMK2::POSITION_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUT"), module, GrainEngineMK2::POSITION_INPUT));

    addParam(createParamCentered<VoxglitchAttenuator>(themePos("JITTER_KNOB"), module, GrainEngineMK2::JITTER_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("JITTER_INPUT"), module, GrainEngineMK2::JITTER_INPUT));

    // Spawn rate
    addParam(createParamCentered<VoxglitchMediumKnob>(themePos("RATE_KNOB"), module, GrainEngineMK2::RATE_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("RATE_ATTN_KNOB"), module, GrainEngineMK2::RATE_ATTN_KNOB));;
    addInput(createInputCentered<VoxglitchInputPort>(themePos("RATE_INPUT"), module, GrainEngineMK2::RATE_INPUT));

    // Window
    addParam(createParamCentered<VoxglitchMediumKnob>(themePos("WINDOW_KNOB"), module, GrainEngineMK2::WINDOW_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("WINDOW_ATTN_KNOB"), module, GrainEngineMK2::WINDOW_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("WINDOW_INPUT"), module, GrainEngineMK2::WINDOW_INPUT));

    // Grains
    addParam(createParamCentered<VoxglitchMediumKnob>(themePos("GRAINS_KNOB"), module, GrainEngineMK2::GRAINS_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("GRAINS_ATTN_KNOB"), module, GrainEngineMK2::GRAINS_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("GRAINS_INPUT"), module, GrainEngineMK2::GRAINS_INPUT));

    // Pitch
    addParam(createParamCentered<VoxglitchMediumKnob>(themePos("PITCH_KNOB"), module, GrainEngineMK2::PITCH_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("PITCH_INPUT"), module, GrainEngineMK2::PITCH_INPUT));

    // Sample
    addParam(createParamCentered<VoxglitchMediumKnob>(themePos("SAMPLE_KNOB"), module, GrainEngineMK2::SAMPLE_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("SAMPLE_ATTN_KNOB"), module, GrainEngineMK2::SAMPLE_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("SAMPLE_INPUT"), module, GrainEngineMK2::SAMPLE_INPUT));

    // Spawn trigger input
    addInput(createInputCentered<VoxglitchInputPort>(themePos("SPAWN_TRIGGER_INPUT"), module, GrainEngineMK2::SPAWN_TRIGGER_INPUT));

    // Pan input
    addInput(createInputCentered<VoxglitchInputPort>(themePos("PAN_INPUT"), module, GrainEngineMK2::PAN_INPUT));

    // Trim knob
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("TRIM_KNOB"), module, GrainEngineMK2::TRIM_KNOB));

    // Audio Output
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_LEFT"), module, GrainEngineMK2::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_RIGHT"), module, GrainEngineMK2::AUDIO_OUTPUT_RIGHT));


    GrainEngineMK2PosDisplay *pos_display = new GrainEngineMK2PosDisplay();
    pos_display->box.pos = themePos("DISPLAY");
    pos_display->module = module;
    addChild(pos_display);
  }

  /*
  struct BipolarPitchOption : MenuItem {
    GrainEngineMK2 *module;

    void onAction(const event::Action &e) override {
      module->bipolar_pitch_mode ^= true; // flip the value
    }
  };
  */

  void appendContextMenu(Menu *menu) override
  {
    GrainEngineMK2 *module = dynamic_cast<GrainEngineMK2*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Samples"));

		//
		// Add the sample slots to the right-click context menu
		//

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			GrainEngineMK2LoadSample *menu_item_load_sample = new GrainEngineMK2LoadSample();
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = std::to_string(i+1) + ": " + module->loaded_filenames[i];
			menu_item_load_sample->module = module;
			menu->addChild(menu_item_load_sample);
		}

  }
};
