struct GrooveBoxExpanderWidget : ModuleWidget
{
  float mute_button_positions[NUMBER_OF_TRACKS][2] = {
    {20, 30},
    {20, 40},
    {20, 50},
    {20, 60},
    {20, 70},
    {20, 80},
    {20, 90},
    {20, 100}
  };

  float solo_button_positions[NUMBER_OF_TRACKS][2] = {
    {45, 30},
    {45, 40},
    {45, 50},
    {45, 60},
    {45, 70},
    {45, 80},
    {45, 90},
    {45, 100}
  };

  GrooveBoxExpanderWidget(GrooveBoxExpander* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groove_box_expander_front_panel.svg")));

    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      float mute_button_x = mute_button_positions[i][0];
      float mute_button_y = mute_button_positions[i][1];

      float solo_button_x = solo_button_positions[i][0];
      float solo_button_y = solo_button_positions[i][1];

      addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(mute_button_x, mute_button_y)), module, GrooveBoxExpander::MUTE_BUTTONS + i, GrooveBoxExpander::MUTE_BUTTON_LIGHTS + i));
      addParam(createLightParamCentered<VCVLightBezel<GreenLight>>(mm2px(Vec(solo_button_x, solo_button_y)), module, GrooveBoxExpander::SOLO_BUTTONS + i, GrooveBoxExpander::SOLO_BUTTON_LIGHTS + i));

      addInput(createInputCentered<PJ301MPort>(mm2px(Vec(mute_button_x - 10, mute_button_y)), module, GrooveBoxExpander::MUTE_INPUTS + i));
      addInput(createInputCentered<PJ301MPort>(mm2px(Vec(solo_button_x - 10, solo_button_y)), module, GrooveBoxExpander::SOLO_INPUTS + i));
    }
  }

};
