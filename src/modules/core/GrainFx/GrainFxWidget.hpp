struct GrainFxWidget : ModuleWidget
{
  GrainFxWidget(GrainFx* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_fx/panel.svg")));

    float y_offset = 1.8;
    float x_offset = -1.8;

    // Jitter
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 0, 45.713)), module, GrainFx::JITTER_CV_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(75.595 + 0, 45.713)), module, GrainFx::JITTER_KNOB));

    // Pan
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 0, 65.759)), module, GrainFx::PAN_INPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(75.595 + 0, 65.759)), module, GrainFx::PAN_SWITCH));

    // Freeze
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366 + 0, 85.805)), module, GrainFx::FREEZE_INPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(75.595 + 0, 85.805)), module, GrainFx::FREEZE_SWITCH));

    //
    // Main Left-side Knobs
    //

    // Spawn rate
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 28.526 - y_offset)), module, GrainFx::SPAWN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 28.526 - y_offset)), module, GrainFx::SPAWN_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 28.526 - y_offset)), module, GrainFx::SPAWN_ATTN_KNOB));;
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(51, 28.526 - y_offset)), module, GrainFx::SPAWN_INDICATOR_LIGHT));

    // Spawn rate override
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(62.366, 28.526 - y_offset)), module, GrainFx::SPAWN_TRIGGER_INPUT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(70.4, 28.526 - y_offset)), module, GrainFx::EXT_CLK_INDICATOR_LIGHT));

    // Grains
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset, 50.489 - y_offset)), module, GrainFx::GRAINS_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 50.489 - y_offset)), module, GrainFx::GRAINS_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26, 50.489 - y_offset)), module, GrainFx::GRAINS_ATTN_KNOB));

    // Window
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 72.452 - y_offset)), module, GrainFx::WINDOW_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 72.452 - y_offset)), module, GrainFx::WINDOW_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 72.452 - y_offset)), module, GrainFx::WINDOW_ATTN_KNOB));

    // Pitch
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44 + x_offset + 0, 94.416 - y_offset)), module, GrainFx::PITCH_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 0, 94.416 - y_offset)), module, GrainFx::PITCH_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(26 + 0, 94.416 - y_offset)), module, GrainFx::PITCH_ATTN_KNOB));

    // Trim
    addParam(createParamCentered<Trimpot>(mm2px(Vec(74.94 + 0, 103.043)), module, GrainFx::TRIM_KNOB));

    // Audio Input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 114.702)), module, GrainFx::AUDIO_INPUT_LEFT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21, 114.702)), module, GrainFx::AUDIO_INPUT_RIGHT));
    addChild(createLightCentered<MediumLight<GreenRedLight>>(mm2px(Vec(32, 114.702)), module, GrainFx::BUFFERING_GREEN_LIGHT));

    // Audio Output
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.216 + 0, 114.702)), module, GrainFx::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94 + 0, 114.702)), module, GrainFx::AUDIO_OUTPUT_RIGHT));


    // Modulation waveform
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(92.743 + 1.403, 20)), module, GrainFx::INTERNAL_MODULATION_WAVEFORM_1_LED));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(99.157 + 1.403, 20)), module, GrainFx::INTERNAL_MODULATION_WAVEFORM_2_LED));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(105.438 + 1.403, 20)), module, GrainFx::INTERNAL_MODULATION_WAVEFORM_3_LED));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(111.719 + 1.403, 20)), module, GrainFx::INTERNAL_MODULATION_WAVEFORM_4_LED));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(118 + 1.403, 20)), module, GrainFx::INTERNAL_MODULATION_WAVEFORM_5_LED));

    // Modulation waveform selection inputs
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(95, 31)), module, GrainFx::INTERNAL_MODULATION_WAVEFORM_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(107, 31)), module, GrainFx::INTERNAL_MODULATION_WAVEFORM_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(118, 31)), module, GrainFx::INTERNAL_MODULATION_WAVEFORM_INPUT));

    // Modulation LFO frequency
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(95, 51.638)), module, GrainFx::INTERNAL_MODULATION_FREQUENCY_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(107, 51.638)), module, GrainFx::INTERNAL_MODULATION_FREQUENCY_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(118, 51.638)), module, GrainFx::INTERNAL_MODULATION_FREQUENCY_INPUT));

    // Modulation LFO Amplitude
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(95, 51 + 20.638)), module, GrainFx::INTERNAL_MODULATION_AMPLITUDE_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(107, 51 + 20.638)), module, GrainFx::INTERNAL_MODULATION_AMPLITUDE_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(118, 51 + 20.638)), module, GrainFx::INTERNAL_MODULATION_AMPLITUDE_INPUT));


    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(118, 88)), module, GrainFx::INTERNAL_MODULATION_OUTPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(105, 88)), module, GrainFx::INTERNAL_MODULATION_OUTPUT_POLARITY_SWITCH));

    // addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.975, 92.0)), module, GrainFx::INTERNAL_MODULATION_BUFFER_OFFSET_KNOB));

    // Position Override
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(95, 114.702)), module, GrainFx::SAMPLE_PLAYBACK_POSITION_KNOB));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(107, 114.702)), module, GrainFx::SAMPLE_PLAYBACK_POSITION_ATTN_KNOB));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(118, 114.702)), module, GrainFx::SAMPLE_PLAYBACK_POSITION_INPUT));
  }

  void appendContextMenu(Menu *menu) override
  {
    GrainFx *module = dynamic_cast<GrainFx*>(this->module);
    assert(module);
  }


};
