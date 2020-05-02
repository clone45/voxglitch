struct GrainBlenderWidget : ModuleWidget
{
  GrainBlenderWidget(GrainBlender* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_blender_front_panel.svg")));

    float y_offset = 1.8;
    float x_offset = -1.8;

    /*
    PanelArt *panel_art = new PanelArt();
    panel_art->module = module;
    addChild(panel_art);
    */

    // Spawn Trigger
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 0, 25.974)), module, GrainBlender::SPAWN_TRIGGER_INPUT));


    // Jitter
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 0, 45.713)), module, GrainBlender::JITTER_CV_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(75.595 + 0, 45.713)), module, GrainBlender::JITTER_KNOB));

    // Pan
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 0, 65.759)), module, GrainBlender::PAN_INPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(75.595 + 0, 65.759)), module, GrainBlender::PAN_SWITCH));

    // Freeze
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 0, 85.805)), module, GrainBlender::FREEZE_INPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(75.595 + 0, 85.805)), module, GrainBlender::FREEZE_SWITCH));

    // Advanced controls
    // addParam(createParamCentered<Trimpot>(mm2px(Vec(32, 72.576)), module, GrainBlender::MAX_WINDOW_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(32, 92.576)), module, GrainBlender::MAX_GRAINS_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(42, 92.576)), module, GrainBlender::CONTOUR_KNOB));


    //
    // Main Left-side Knobs
    //

    // Position
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 28.526 - y_offset)), module, GrainBlender::SAMPLE_PLAYBACK_POSITION_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 28.526 - y_offset)), module, GrainBlender::SAMPLE_PLAYBACK_POSITION_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 28.526 - y_offset)), module, GrainBlender::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));

    // Pitch
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 50.489 - y_offset)), module, GrainBlender::PITCH_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 50.489 - y_offset)), module, GrainBlender::PITCH_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 50.489 - y_offset)), module, GrainBlender::PITCH_ATTN_KNOB));

    // Window
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 72.452 - y_offset)), module, GrainBlender::WINDOW_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 72.452 - y_offset)), module, GrainBlender::WINDOW_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 72.452 - y_offset)), module, GrainBlender::WINDOW_ATTN_KNOB));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 94.415 - y_offset)), module, GrainBlender::SPAWN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 94.415 - y_offset)), module, GrainBlender::SPAWN_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 94.415 - y_offset)), module, GrainBlender::SPAWN_ATTN_KNOB));;

    // Trim
    addParam(createParamCentered<Trimpot>(mm2px(Vec(74.94 + 0, 103.043)), module, GrainBlender::TRIM_KNOB));

    // Audio Input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 114.702)), module, GrainBlender::AUDIO_INPUT_LEFT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21, 114.702)), module, GrainBlender::AUDIO_INPUT_RIGHT));

    // Audio Output
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.216 + 0, 114.702)), module, GrainBlender::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94 + 0, 114.702)), module, GrainBlender::AUDIO_OUTPUT_RIGHT));


    // Modulation area
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(100 + 0, 85.805)), module, GrainBlender::INTERNAL_MODULATION_FREQUENCY_KNOB));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(100 + 0, 94.415)), module, GrainBlender::INTERNAL_MODULATION_AMPLITUDE_KNOB));


  }

  void appendContextMenu(Menu *menu) override
  {
    GrainBlender *module = dynamic_cast<GrainBlender*>(this->module);
    assert(module);
  }


};
