struct ByteBenderWidget : ModuleWidget
{
  ByteBenderWidget(ByteBender* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/bytebender_front_panel.svg")));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(50, 50)), module, ByteBender::T1_OUTPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60, 50)), module, ByteBender::INPUT_1A));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60, 60)), module, ByteBender::INPUT_1B));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(60, 70)), module, ByteBender::OUTPUT_1));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(70, 50)), module, ByteBender::INPUT_2A));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(70, 60)), module, ByteBender::INPUT_2B));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(70, 70)), module, ByteBender::OUTPUT_2));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(80, 50)), module, ByteBender::INPUT_3A));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(80, 60)), module, ByteBender::INPUT_3B));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(80, 70)), module, ByteBender::OUTPUT_3));

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
