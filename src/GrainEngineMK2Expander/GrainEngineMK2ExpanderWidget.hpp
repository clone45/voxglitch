struct GrainEngineMK2ExpanderWidget : ModuleWidget
{
  GrainEngineMK2ExpanderWidget(GrainEngineMK2Expander* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_mk2_expander_front_panel.svg")));

    float col_1 = 9.878;
    float col_2 = 20.602;

    float row_distance = 19.592;

    float row_1 = 35.612 - 8;  // 35.612 - 6.264 =
    float row_2 = row_1 + row_distance;
    float row_3 = row_2 + row_distance;
    float row_4 = row_3 + row_distance;

    // Row 1
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col_1, row_1)), module, GrainEngineMK2Expander::AUDIO_IN_LEFT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col_2, row_1)), module, GrainEngineMK2Expander::AUDIO_IN_RIGHT));

    // Row 2
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col_1, row_2)), module, GrainEngineMK2Expander::RECORD_START_INPUT));
    addParam(createParamCentered<LEDButton>(mm2px(Vec(col_2, row_2)), module, GrainEngineMK2Expander::RECORD_START_BUTTON_PARAM));
    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(col_2, row_2)), module, GrainEngineMK2Expander::RECORDING_LIGHT));

    // Row 3
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col_1, row_3)), module, GrainEngineMK2Expander::RECORD_STOP_INPUT));
    addParam(createParamCentered<LEDButton>(mm2px(Vec(col_2, row_3)), module, GrainEngineMK2Expander::RECORD_STOP_BUTTON_PARAM));
    addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(col_2, row_3)), module, GrainEngineMK2Expander::STOPPED_LIGHT));

    // Row 4
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col_1, row_4)), module, GrainEngineMK2Expander::SAMPLE_SLOT_INPUT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(col_2, row_4)), module, GrainEngineMK2Expander::SAMPLE_SLOT_KNOB_PARAM));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col_1, 114.702)), module, GrainEngineMK2Expander::PASSTHROUGH_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col_2, 114.702)), module, GrainEngineMK2Expander::PASSTHROUGH_RIGHT));
  }

  void appendContextMenu(Menu *menu) override
  {
    GrainEngineMK2Expander *module = dynamic_cast<GrainEngineMK2Expander*>(this->module);
    assert(module);
  }
};
