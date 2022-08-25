struct SatanonautWidget : VoxglitchModuleWidget
{
  SatanonautWidget(Satanonaut* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/satanonaut/satanonaut_front_panel.svg")));

    ImageWidget *png_panel = new ImageWidget("res/satanonaut/satanonaut_baseplate.png", 152.4, 128.5);
    addChild(png_panel);

    // add_snapping_parameter_knob(COLUMN_15, ROW_7 + 8, Satanonaut::EFFECT_KNOB);

    addParam(createParamCentered<PainfulMediumKnob>(Vec(415.025452,67.072342), module, Satanonaut::EFFECT_KNOB));
    addParam(createParamCentered<PainfulMediumKnob>(Vec(415.025452,107.922394), module, Satanonaut::BUFFER_SIZE_KNOB));
    addParam(createParamCentered<PainfulMediumKnob>(Vec(415.025452,147.922394), module, Satanonaut::FEEDBACK_KNOB));
    addParam(createParamCentered<PainfulMediumKnob>(Vec(415.025452,187.922394), module, Satanonaut::PARAM_1_KNOB));
    addParam(createParamCentered<PainfulMediumKnob>(Vec(415.025452,227.922394), module, Satanonaut::PARAM_2_KNOB));

    //
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_2)), module, Satanonaut::BUFFER_SIZE_INPUT));
    // addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_7 + 8)), module, Satanonaut::EFFECT_KNOB));
    /*
    add_snapping_parameter_knob(COLUMN_15, ROW_7 + 8, Satanonaut::EFFECT_KNOB);
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_9 + 4)), module, Satanonaut::BUFFER_SIZE_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_11)), module, Satanonaut::FEEDBACK_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_13 - 4)), module, Satanonaut::PARAM_1_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_15 - 8)), module, Satanonaut::PARAM_2_KNOB));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_7 + 8)), module, Satanonaut::EFFECT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_9 + 4)), module, Satanonaut::BUFFER_SIZE_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_11)), module, Satanonaut::FEEDBACK_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_13 - 4)), module, Satanonaut::PARAM_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_15 - 8)), module, Satanonaut::PARAM_2_INPUT));
    */

    addInput(createInputCentered<PJ301MPort>(Vec(374.5, 68.5), module, Satanonaut::EFFECT_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(374.5, 109.0), module, Satanonaut::BUFFER_SIZE_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(375.0, 149.0), module, Satanonaut::FEEDBACK_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(375.0, 190.0), module, Satanonaut::PARAM_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(375.0, 230.0), module, Satanonaut::PARAM_2_INPUT));

    // Drive knob
    addParam(createParamCentered<VoxglitchValve>(Vec(326.587891,296.511719), module, Satanonaut::DRIVE_KNOB));

    // Inputs and outputs

    addInput(createInputCentered<PJ301MPort>(Vec(34, 292), module, Satanonaut::AUDIO_INPUT_LEFT));
    addInput(createInputCentered<PJ301MPort>(Vec(34, 326), module, Satanonaut::AUDIO_INPUT_RIGHT));

    addOutput(createOutputCentered<PJ301MPort>(Vec(415, 292), module, Satanonaut::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(415, 325), module, Satanonaut::AUDIO_OUTPUT_RIGHT));
  }

  void add_snapping_parameter_knob(float column, float row, int index)
  {
    auto P = createParamCentered<PainfulMediumKnob>(mm2px(Vec(column, row)), module, index);
    dynamic_cast<Knob*>(P)->snap = true;
    addParam(P);
  }
};
