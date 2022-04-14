struct GrainEngineMK2Widget : ModuleWidget
{
  GrainEngineMK2Widget(GrainEngineMK2* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_mk2/grain_engine_mk2_front_panel.svg")));

    PNGPanel *png_panel = new PNGPanel("res/grain_engine_mk2/grain_engine_main_baseplate.png", 101.6, 128.5);
    addChild(png_panel);

    // Add typography layer
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_mk2/grain_engine_typography.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);

    //
    // Control inputs in the center area of the module
    //
    addParam(createParamCentered<VoxglitchLargeKnob>(Vec(150.407593,78.039162), module, GrainEngineMK2::POSITION_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(224.822815,59.589138), module, GrainEngineMK2::POSITION_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(231.972626,96.289116), module, GrainEngineMK2::POSITION_INPUT));

    addParam(createParamCentered<VoxglitchAttenuator>(Vec(75.375320,59.539215), module, GrainEngineMK2::JITTER_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(67.875175,96.400185), module, GrainEngineMK2::JITTER_INPUT));

    // Spawn rate
    addParam(createParamCentered<VoxglitchMediumKnob>(Vec(33.867897,178.221176), module, GrainEngineMK2::RATE_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(33.867897,223.223434), module, GrainEngineMK2::RATE_ATTN_KNOB));;
    addInput(createInputCentered<VoxglitchInputPort>(Vec(33.867897,260.573914), module, GrainEngineMK2::RATE_INPUT));

    // Window
    addParam(createParamCentered<VoxglitchMediumKnob>(Vec(91.929710, 178.221176), module, GrainEngineMK2::WINDOW_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(91.929710, 223.223434), module, GrainEngineMK2::WINDOW_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(91.929710, 260.573914), module, GrainEngineMK2::WINDOW_INPUT));

    // Grains
    addParam(createParamCentered<VoxglitchMediumKnob>(Vec(149.991523, 178.221176), module, GrainEngineMK2::GRAINS_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(149.991523, 223.223434), module, GrainEngineMK2::GRAINS_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(149.991523, 260.573914), module, GrainEngineMK2::GRAINS_INPUT));

    // Pitch
    addParam(createParamCentered<VoxglitchMediumKnob>(Vec(208.303345, 178.221176), module, GrainEngineMK2::PITCH_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(208.303345, 223.223434), module, GrainEngineMK2::PITCH_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(208.303345, 260.573914), module, GrainEngineMK2::PITCH_INPUT));

    // Sample
    addParam(createParamCentered<VoxglitchMediumKnob>(Vec(266.415070, 178.221176), module, GrainEngineMK2::SAMPLE_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(266.415070, 223.223434), module, GrainEngineMK2::SAMPLE_ATTN_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(266.415070, 260.573914), module, GrainEngineMK2::SAMPLE_INPUT));

    // Spawn trigger input
    addInput(createInputCentered<VoxglitchInputPort>(Vec(33.607643,342.537292), module, GrainEngineMK2::SPAWN_TRIGGER_INPUT));

    // Pan input
    addInput(createInputCentered<VoxglitchInputPort>(Vec(91.887779,342.637268), module, GrainEngineMK2::PAN_INPUT));

    // Trim knob
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(180.087967,342.287354), module, GrainEngineMK2::TRIM_KNOB));

    // Audio Output
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(225.577179,342.537292), module, GrainEngineMK2::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(266.140045,342.452545), module, GrainEngineMK2::AUDIO_OUTPUT_RIGHT));


    GrainEngineMK2PosDisplay *pos_display = new GrainEngineMK2PosDisplay();
    pos_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    pos_display->module = module;
    addChild(pos_display);
  }

  struct BipolarPitchOption : MenuItem {
    GrainEngineMK2 *module;

    void onAction(const event::Action &e) override {
      module->bipolar_pitch_mode ^= true; // flip the value
    }
  };

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

    // Bipolar pitch mode selection
    BipolarPitchOption *bipolar_pitch_option = createMenuItem<BipolarPitchOption>("Bipolar Pitch CV Input", CHECKMARK(module->bipolar_pitch_mode));
    bipolar_pitch_option->module = module;
    menu->addChild(bipolar_pitch_option);
  }
};
