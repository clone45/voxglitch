struct GrainEngineMK2Widget : ModuleWidget
{
  GrainEngineMK2Widget(GrainEngineMK2* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_mk2_front_panel_r2.svg")));

    float hrule1 = 128.5 - 104.035;
    float hrule2 = 128.5 - 92.415;
    float hrule3 = 128.5 - 63.689;
    float hrule4 = 128.5 - 49.515;
    float hrule5 = 128.5 - 38.034;
    float hrule6 = 128.5 - 11.977;

    float vrule_a_1 = 15.903;
    float vrule_a_2 = 37.278;
    float vrule_a_3 = 53.263;
    float vrule_a_4 = 69.249;
    float vrule_a_5 = 85.234;

    float vrule_b_1 = 9.421;
    float vrule_b_2 = 29.491;
    float vrule_b_3 = 49.562;
    float vrule_b_4 = 69.632;
    float vrule_b_5 = 89.702;

    // Position
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(vrule_a_1, hrule1)), module, GrainEngineMK2::POSITION_COARSE_KNOB));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_a_2, hrule1)), module, GrainEngineMK2::POSITION_COARSE_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_a_2, hrule2)), module, GrainEngineMK2::POSITION_COARSE_INPUT));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_a_3, hrule1)), module, GrainEngineMK2::POSITION_MEDIUM_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_a_3, hrule2)), module, GrainEngineMK2::POSITION_MEDIUM_INPUT));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_a_4, hrule1)), module, GrainEngineMK2::POSITION_FINE_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_a_4, hrule2)), module, GrainEngineMK2::POSITION_FINE_INPUT));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_a_5, hrule1)), module, GrainEngineMK2::JITTER_KNOB));    
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_a_5, hrule2)), module, GrainEngineMK2::JITTER_CV_INPUT));


    /*
    // Jitter

    // Pan
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, hrule3)), module, GrainEngineMK2::PAN_INPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(75.595, hrule3)), module, GrainEngineMK2::PAN_SWITCH));

    // Spawn rate override
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, hrule1)), module, GrainEngineMK2::SPAWN_TRIGGER_INPUT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(70.4, hrule1)), module, GrainEngineMK2::EXT_CLK_INDICATOR_LIGHT));



    // Position Fine input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, hrule2)), module, GrainEngineMK2::POSITION_FINE_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(75.595, hrule2)), module, GrainEngineMK2::POSITION_FINE_ATTN_KNOB));
    */

    //
    // Control inputs in the center area of the module
    //

    // Spawn rate
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(vrule_b_1, hrule3)), module, GrainEngineMK2::SPAWN_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_b_1, hrule4)), module, GrainEngineMK2::SPAWN_ATTN_KNOB));;
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_b_1, hrule5)), module, GrainEngineMK2::SPAWN_INPUT));

    // Window
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(vrule_b_2, hrule3)), module, GrainEngineMK2::WINDOW_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_b_2, hrule4)), module, GrainEngineMK2::WINDOW_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_b_2, hrule5)), module, GrainEngineMK2::WINDOW_INPUT));

    // Grains
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(vrule_b_3, hrule3)), module, GrainEngineMK2::GRAINS_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_b_3, hrule4)), module, GrainEngineMK2::GRAINS_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_b_3, hrule5)), module, GrainEngineMK2::GRAINS_INPUT));

    // Pitch
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(vrule_b_4, hrule3)), module, GrainEngineMK2::PITCH_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_b_4, hrule4)), module, GrainEngineMK2::PITCH_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_b_4, hrule5)), module, GrainEngineMK2::PITCH_INPUT));

    // Sample
    /*
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(vrule_b_5, hrule3)), module, GrainEngineMK2::SAMPLE_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_b_5, hrule4)), module, GrainEngineMK2::SAMPLE_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_b_5, hrule5)), module, GrainEngineMK2::SAMPLE_INPUT));
    */

    // Trim
    addParam(createParamCentered<Trimpot>(mm2px(Vec(74.94, 103.043)), module, GrainEngineMK2::TRIM_KNOB));

    // Audio Output
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.216, 114.702)), module, GrainEngineMK2::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94, 114.702)), module, GrainEngineMK2::AUDIO_OUTPUT_RIGHT));

    // Position Override
    // addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(95, 114.702)), module, GrainEngineMK2::POSITION_KNOB));
    // addParam(createParamCentered<Trimpot>(mm2px(Vec(107, 114.702)), module, GrainEngineMK2::POSITION_ATTN_KNOB));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(118, 114.702)), module, GrainEngineMK2::POSITION_INPUT));
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
