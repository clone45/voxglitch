struct InnerWidget : VoxglitchModuleWidget
{
  InnerWidget(Inner *module)
  {
    setModule(module);

    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/inner_front_panel.svg")));
  }
};
