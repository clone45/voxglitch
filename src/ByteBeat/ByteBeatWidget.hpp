struct ByteBeatWidget : VoxglitchModuleWidget
{
  ByteBeatWidget(ByteBeat* module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("bytebeat");
    applyTheme();

    // setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/bytebeat/panel.svg")));

		setPanel(createPanel(
			asset::plugin(pluginInstance, "res/bytebeat/bytebeat_panel.svg"),
			asset::plugin(pluginInstance, "res/bytebeat/bytebeat_panel-dark.svg")
		));

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
    auto L1 = createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(17.0, 27.0)), module, ByteBeat::EQUATION_KNOB); dynamic_cast<Knob*>(L1)->snap = true; addParam(L1);
    addInput(createInputCentered<VoxglitchInputPort>(mm2px(Vec(32.35, ROW_5)), module, ByteBeat::EQUATION_INPUT));

    // Parameter inputs
    auto P1 = createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(51.8, ROW_3)), module, ByteBeat::PARAM_KNOB_1); dynamic_cast<Knob*>(P1)->snap = true; addParam(P1);
    addInput(createInputCentered<VoxglitchInputPort>(mm2px(Vec(51.8, ROW_5)), module, ByteBeat::PARAM_INPUT_1));

    auto P2 = createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(69.85, ROW_3)), module, ByteBeat::PARAM_KNOB_2); dynamic_cast<Knob*>(P2)->snap = true; addParam(P2);
    addInput(createInputCentered<VoxglitchInputPort>(mm2px(Vec(69.85, ROW_5)), module, ByteBeat::PARAM_INPUT_2));

    auto P3 = createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(88.0, ROW_3)), module, ByteBeat::PARAM_KNOB_3); dynamic_cast<Knob*>(P3)->snap = true; addParam(P3);
    addInput(createInputCentered<VoxglitchInputPort>(mm2px(Vec(88.0, ROW_5)), module, ByteBeat::PARAM_INPUT_3));

    // Other
    addOutput(createOutputCentered<VoxglitchOutputPort>(mm2px(Vec(88.9, 112.4375)), module, ByteBeat::AUDIO_OUTPUT));

    // Pitch
    addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PITCH_KNOB"), module, ByteBeat::CLOCK_DIVISION_KNOB));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("PITCH_INPUT"), module, ByteBeat::CLOCK_CV_INPUT));

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
