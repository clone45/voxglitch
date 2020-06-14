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

    // float vrule_c_1 = 21.448;
    float vrule_c_2 = 35.969;
    float vrule_c_3 = 50.491;
    float vrule_c_4 = 65.012;

    // Position
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(vrule_a_1, hrule1)), module, GrainEngineMK2::POSITION_COARSE_KNOB));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_a_2, hrule1)), module, GrainEngineMK2::POSITION_COARSE_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_a_2, hrule2)), module, GrainEngineMK2::POSITION_COARSE_INPUT));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_a_3, hrule1)), module, GrainEngineMK2::POSITION_MEDIUM_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_a_3, hrule2)), module, GrainEngineMK2::POSITION_MEDIUM_INPUT));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_a_4, hrule1)), module, GrainEngineMK2::POSITION_FINE_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_a_4, hrule2)), module, GrainEngineMK2::POSITION_FINE_INPUT));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_a_5, hrule1)), module, GrainEngineMK2::JITTER_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_a_5, hrule2)), module, GrainEngineMK2::JITTER_INPUT));

    // Spawn input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_c_2, hrule6)), module, GrainEngineMK2::SPAWN_TRIGGER_INPUT));

    // Pan input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_c_3, hrule6)), module, GrainEngineMK2::PAN_INPUT));

    // Trim knob
    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_c_4, hrule6)), module, GrainEngineMK2::TRIM_KNOB));

    // Audio Output
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(79.816, hrule6)), module, GrainEngineMK2::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(90.54, hrule6)), module, GrainEngineMK2::AUDIO_OUTPUT_RIGHT));

    //
    // Control inputs in the center area of the module
    //

    // Spawn rate
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(vrule_b_1, hrule3)), module, GrainEngineMK2::RATE_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_b_1, hrule4)), module, GrainEngineMK2::RATE_ATTN_KNOB));;
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_b_1, hrule5)), module, GrainEngineMK2::RATE_INPUT));

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
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(vrule_b_5, hrule3)), module, GrainEngineMK2::SAMPLE_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(vrule_b_5, hrule4)), module, GrainEngineMK2::SAMPLE_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(vrule_b_5, hrule5)), module, GrainEngineMK2::SAMPLE_INPUT));
  }

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
