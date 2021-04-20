struct GalactoWidget : ModuleWidget
{
  GalactoWidget(Galacto* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/galacto_front_panel.svg")));


    //
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_2)), module, Galacto::BUFFER_SIZE_INPUT));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_7 + 8)), module, Galacto::EFFECT_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_9 + 4)), module, Galacto::BUFFER_SIZE_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_11)), module, Galacto::FEEDBACK_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_13 - 4)), module, Galacto::PARAM_2_KNOB));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(COLUMN_15, ROW_15 - 8)), module, Galacto::PARAM_2_KNOB));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_7 + 8)), module, Galacto::EFFECT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_9 + 4)), module, Galacto::BUFFER_SIZE_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_11)), module, Galacto::FEEDBACK_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_13 - 4)), module, Galacto::PARAM_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_13 + 5, ROW_15 - 8)), module, Galacto::PARAM_2_INPUT));



    // Other
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_1, ROW_15)), module, Galacto::AUDIO_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_3, ROW_15)), module, Galacto::AUDIO_OUTPUT));

    GalactoEffectReadout *effect_readout = new GalactoEffectReadout();
		effect_readout->box.pos = mm2px(Vec(8, 64.3));
		effect_readout->box.size = Vec(200, 30); // bounding box of the widget
		effect_readout->module = module;
		addChild(effect_readout);

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_15 + 21, ROW_1)), module, Galacto::DEBUG_OUTPUT));
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
