struct InnerWidget : VoxglitchModuleWidget
{
  InnerWidget(Inner *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/inner_front_panel.svg")));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 50.702)), module, Inner::PARAM1_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 70.702)), module, Inner::PARAM2_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 90.702)), module, Inner::PARAM3_CV_INPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.94, 70.702)), module, Inner::PITCH_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.94, 90.702)), module, Inner::GATE_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94, 114.702)), module, Inner::AUDIO_OUTPUT));
  }
};