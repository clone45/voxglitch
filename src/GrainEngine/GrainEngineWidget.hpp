struct GrainEngineWidget : ModuleWidget
{
  GrainEngineWidget(GrainEngine* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_front_panel.svg")));

    float y_offset = 1.8;
    float x_offset = -1.8;

    // Spawn Trigger
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, 25.974)), module, GrainEngine::SPAWN_TRIGGER_INPUT));

    // Jitter
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, 45.713)), module, GrainEngine::JITTER_CV_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(75.595, 45.713)), module, GrainEngine::JITTER_KNOB));

    // Pan
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, 65.759)), module, GrainEngine::PAN_INPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(75.595, 65.759)), module, GrainEngine::PAN_SWITCH));

    //
    // Main Left-side Knobs
    //

    // Position
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 28.526 - y_offset)), module, GrainEngine::SAMPLE_PLAYBACK_POSITION_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 28.526 - y_offset)), module, GrainEngine::SAMPLE_PLAYBACK_POSITION_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 28.526 - y_offset)), module, GrainEngine::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

    // Pitch
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 50.489 - y_offset)), module, GrainEngine::PITCH_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 50.489 - y_offset)), module, GrainEngine::PITCH_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 50.489 - y_offset)), module, GrainEngine::PITCH_ATTN_KNOB));

    // Amp Slope
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 72.452 - y_offset)), module, GrainEngine::AMP_SLOPE_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 72.452 - y_offset)), module, GrainEngine::AMP_SLOPE_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 72.452 - y_offset)), module, GrainEngine::AMP_SLOPE_ATTN_KNOB));

    // Length
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 94.416 - y_offset)), module, GrainEngine::LENGTH_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 94.416 - y_offset)), module, GrainEngine::LENGTH_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 94.416 - y_offset)), module, GrainEngine::LENGTH_ATTN_KNOB));

    // Len Mult Knob
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 110)), module, GrainEngine::LEN_MULT_KNOB));

    // Trim
    addParam(createParamCentered<Trimpot>(mm2px(Vec(74.94, 103.043)), module, GrainEngine::TRIM_KNOB));

    // WAV output

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.216, 114.702)), module, GrainEngine::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94, 114.702)), module, GrainEngine::AUDIO_OUTPUT_RIGHT));

  }

  void appendContextMenu(Menu *menu) override
  {
    GrainEngine *module = dynamic_cast<GrainEngine*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Sample"));

    GrainEngineLoadSample *menu_item_load_sample = new GrainEngineLoadSample();
    menu_item_load_sample->text = module->loaded_filename;
    menu_item_load_sample->module = module;
    menu->addChild(menu_item_load_sample);
  }

};
