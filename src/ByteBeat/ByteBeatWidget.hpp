struct ByteBeatWidget : ModuleWidget
{
  ByteBeatWidget(ByteBeat* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/bytebeat_front_panel.svg")));

    // Equation inputs

    addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(COLUMN_3, ROW_3 AND_A_HALF_ROW)), module, ByteBeat::EQUATION_KNOB));

    // Expression inputs

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(COLUMN_8, ROW_3)), module, ByteBeat::EXPRESSION_KNOB_1));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_8, ROW_5)), module, ByteBeat::EXPRESSION_INPUT_1));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(COLUMN_11, ROW_3)), module, ByteBeat::EXPRESSION_KNOB_2));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_11, ROW_5)), module, ByteBeat::EXPRESSION_INPUT_2));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(COLUMN_14, ROW_3)), module, ByteBeat::EXPRESSION_KNOB_3));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_5)), module, ByteBeat::EXPRESSION_INPUT_3));

    // Parameter inputs

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(COLUMN_8, ROW_8)), module, ByteBeat::PARAM_KNOB_1));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_8, ROW_10)), module, ByteBeat::PARAM_INPUT_1));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(COLUMN_11, ROW_8)), module, ByteBeat::PARAM_KNOB_2));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_11, ROW_10)), module, ByteBeat::PARAM_INPUT_2));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(COLUMN_14, ROW_8)), module, ByteBeat::PARAM_KNOB_3));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_10)), module, ByteBeat::PARAM_INPUT_3));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_14)), module, ByteBeat::AUDIO_OUTPUT));
    // addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_12, ROW_14)), module, ByteBeat::DEBUG_OUTPUT));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(COLUMN_3, ROW_7)), module, ByteBeat::CLOCK_DIVISION_KNOB));
  }
};
