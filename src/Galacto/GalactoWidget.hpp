struct GalactoWidget : ModuleWidget
{
  GalactoWidget(Galacto* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/galacto_front_panel.svg")));

    //
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_2)), module, Galacto::BUFFER_SIZE_INPUT));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(COLUMN_14, ROW_2)), module, Galacto::BUFFER_SIZE_KNOB));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(COLUMN_15, ROW_2)), module, Galacto::FEEDBACK_KNOB));

    // Other
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_6 - 4.2)), module, Galacto::AUDIO_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_15 + 21, ROW_6 - 4.2)), module, Galacto::AUDIO_OUTPUT));

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
