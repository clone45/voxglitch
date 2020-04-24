struct GrainBlenderWidget : ModuleWidget
{
  GrainBlenderWidget(GrainBlender* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_blender_front_panel.svg")));

    float y_offset = 1.8;
    float x_offset = -1.8;

    // Spawn Trigger
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 127.26, 25.974)), module, GrainBlender::SPAWN_TRIGGER_INPUT));

    // Jitter
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 127.26, 45.713)), module, GrainBlender::JITTER_CV_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(75.595 + 127.26, 45.713)), module, GrainBlender::JITTER_KNOB));

    // Pan
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 127.26, 65.759)), module, GrainBlender::PAN_INPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(75.595 + 127.26, 65.759)), module, GrainBlender::PAN_SWITCH));

    //
    // Main Left-side Knobs
    //

    // Position
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 127.26, 28.526 - y_offset)), module, GrainBlender::SAMPLE_PLAYBACK_POSITION_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 127.26, 28.526 - y_offset)), module, GrainBlender::SAMPLE_PLAYBACK_POSITION_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 127.26, 28.526 - y_offset)), module, GrainBlender::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

    // Pitch
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 127.26, 50.489 - y_offset)), module, GrainBlender::PITCH_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 127.26, 50.489 - y_offset)), module, GrainBlender::PITCH_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 127.26, 50.489 - y_offset)), module, GrainBlender::PITCH_ATTN_KNOB));

    // Amp Slope
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 127.26, 72.452 - y_offset)), module, GrainBlender::AMP_SLOPE_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 127.26, 72.452 - y_offset)), module, GrainBlender::AMP_SLOPE_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 127.26, 72.452 - y_offset)), module, GrainBlender::AMP_SLOPE_ATTN_KNOB));

    // Length
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 127.26, 94.416 - y_offset)), module, GrainBlender::LENGTH_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 127.26, 94.416 - y_offset)), module, GrainBlender::LENGTH_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 127.26, 94.416 - y_offset)), module, GrainBlender::LENGTH_ATTN_KNOB));

    // Len Mult Knob
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 127.26, 110)), module, GrainBlender::LEN_MULT_KNOB));

    // Trim
    addParam(createParamCentered<Trimpot>(mm2px(Vec(74.94 + 127.26, 103.043)), module, GrainBlender::TRIM_KNOB));

    // WAV output

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.216 + 127.26, 114.702)), module, GrainBlender::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94 + 127.26, 114.702)), module, GrainBlender::AUDIO_OUTPUT_RIGHT));

  }

  void appendContextMenu(Menu *menu) override
  {
    GrainBlender *module = dynamic_cast<GrainBlender*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Sample"));

    GrainBlenderLoadSample *menu_item_load_sample = new GrainBlenderLoadSample();
    menu_item_load_sample->text = module->loaded_filename;
    menu_item_load_sample->module = module;
    menu->addChild(menu_item_load_sample);
  }

};
