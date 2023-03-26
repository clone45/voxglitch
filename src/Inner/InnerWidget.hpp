struct InnerWidget : VoxglitchModuleWidget
{
  InnerWidget(Inner *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/inner_front_panel.svg")));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94, 114.702)), module, Inner::AUDIO_OUTPUT));
  }
};