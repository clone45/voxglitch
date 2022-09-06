struct ByteBeatWidget : VoxglitchModuleWidget
{
  ByteBeatWidget(ByteBeat* module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("bytebeat");
    applyTheme();

    // =================== PLACE COMPONENTS ====================================

    if(theme.showScrews())
    {
  		addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, 0)));
  		addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  		addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  		addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    // Equation inputs

    // addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(COLUMN_3, ROW_3 AND_A_HALF_ROW)), module, ByteBeat::EQUATION_KNOB));
    auto L1 = createParamCentered<VoxglitchLargeKnob>(themePos("EQUATION_KNOB"), module, ByteBeat::EQUATION_KNOB); dynamic_cast<Knob*>(L1)->snap = true; addParam(L1);
    addInput(createInputCentered<VoxglitchInputPort>(themePos("EQUATION_INPUT"), module, ByteBeat::EQUATION_INPUT));

    // Parameter inputs
    auto P1 = createParamCentered<VoxglitchMediumKnob>(themePos("PARAM_KNOB_1"), module, ByteBeat::PARAM_KNOB_1); dynamic_cast<Knob*>(P1)->snap = true; addParam(P1);
    addInput(createInputCentered<VoxglitchInputPort>(themePos("PARAM_INPUT_1"), module, ByteBeat::PARAM_INPUT_1));

    auto P2 = createParamCentered<VoxglitchMediumKnob>(themePos("PARAM_KNOB_2"), module, ByteBeat::PARAM_KNOB_2); dynamic_cast<Knob*>(P2)->snap = true; addParam(P2);
    addInput(createInputCentered<VoxglitchInputPort>(themePos("PARAM_INPUT_2"), module, ByteBeat::PARAM_INPUT_2));

    auto P3 = createParamCentered<VoxglitchMediumKnob>(themePos("PARAM_KNOB_3"), module, ByteBeat::PARAM_KNOB_3); dynamic_cast<Knob*>(P3)->snap = true; addParam(P3);
    addInput(createInputCentered<VoxglitchInputPort>(themePos("PARAM_INPUT_3"), module, ByteBeat::PARAM_INPUT_3));

    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT"), module, ByteBeat::AUDIO_OUTPUT));

    // addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(COLUMN_3, ROW_7)), module, ByteBeat::CLOCK_DIVISION_KNOB));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.75, 64.25)), module, ByteBeat::CLOCK_CV_INPUT));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_5, ROW_11)), module, ByteBeat::T_INPUT));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_5, ROW_13)), module, ByteBeat::SYNC_CLOCK_INPUT));

  }

  /*
  void add_snapping_parameter_knob(float column, float row, int index)
  {
    auto P = createParamCentered<RoundBlackKnob>(mm2px(Vec(column, row)), module, index);
    dynamic_cast<Knob*>(P)->snap = true;
    addParam(P);
  }
  */
};
