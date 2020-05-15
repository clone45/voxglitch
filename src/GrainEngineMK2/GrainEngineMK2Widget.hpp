struct GrainEngineMK2Widget : ModuleWidget
{
  GrainEngineMK2Widget(GrainEngineMK2* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_mk2_front_panel.svg")));

    float y_offset = 1.8;
    float x_offset = -1.8;

    // Jitter
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 0, 45.713)), module, GrainEngineMK2::JITTER_CV_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(75.595 + 0, 45.713)), module, GrainEngineMK2::JITTER_KNOB));

    // Pan
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 0, 65.759)), module, GrainEngineMK2::PAN_INPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(75.595 + 0, 65.759)), module, GrainEngineMK2::PAN_SWITCH));


    //
    // Main Left-side Knobs
    //

    // Spawn rate
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 28.526 - y_offset)), module, GrainEngineMK2::SPAWN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 28.526 - y_offset)), module, GrainEngineMK2::SPAWN_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 28.526 - y_offset)), module, GrainEngineMK2::SPAWN_ATTN_KNOB));;
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(51, 28.526 - y_offset)), module, GrainEngineMK2::SPAWN_INDICATOR_LIGHT));

    // Spawn rate override
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, 28.526 - y_offset)), module, GrainEngineMK2::SPAWN_TRIGGER_INPUT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(70.4, 28.526 - y_offset)), module, GrainEngineMK2::EXT_CLK_INDICATOR_LIGHT));

    // Grains
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 50.489 - y_offset)), module, GrainEngineMK2::GRAINS_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 50.489 - y_offset)), module, GrainEngineMK2::GRAINS_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 50.489 - y_offset)), module, GrainEngineMK2::GRAINS_ATTN_KNOB));

    // Window
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 72.452 - y_offset)), module, GrainEngineMK2::WINDOW_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 72.452 - y_offset)), module, GrainEngineMK2::WINDOW_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 72.452 - y_offset)), module, GrainEngineMK2::WINDOW_ATTN_KNOB));

    // Pitch
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 94.416 - y_offset)), module, GrainEngineMK2::PITCH_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 94.416 - y_offset)), module, GrainEngineMK2::PITCH_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 94.416 - y_offset)), module, GrainEngineMK2::PITCH_ATTN_KNOB));

    // Position
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 116.634 - y_offset)), module, GrainEngineMK2::SAMPLE_PLAYBACK_POSITION_KNOB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 116.634 - y_offset)), module, GrainEngineMK2::SAMPLE_PLAYBACK_POSITION_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 116.634 - y_offset)), module, GrainEngineMK2::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

    // Trim
    addParam(createParamCentered<Trimpot>(mm2px(Vec(74.94 + 0, 103.043)), module, GrainEngineMK2::TRIM_KNOB));

    // Audio Output
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.216 + 0, 114.702)), module, GrainEngineMK2::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94 + 0, 114.702)), module, GrainEngineMK2::AUDIO_OUTPUT_RIGHT));

    // Position Override
    // addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(95, 114.702)), module, GrainEngineMK2::SAMPLE_PLAYBACK_POSITION_KNOB));
    // addParam(createParamCentered<Trimpot>(mm2px(Vec(107, 114.702)), module, GrainEngineMK2::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(118, 114.702)), module, GrainEngineMK2::SAMPLE_PLAYBACK_POSITION_INPUT));
  }

  void appendContextMenu(Menu *menu) override
  {
    GrainEngineMK2 *module = dynamic_cast<GrainEngineMK2*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Sample"));

    GrainEngineMK2LoadSample *menu_item_load_sample = new GrainEngineMK2LoadSample();
    menu_item_load_sample->text = module->loaded_filename;
    menu_item_load_sample->module = module;
    menu->addChild(menu_item_load_sample);
  }
};
