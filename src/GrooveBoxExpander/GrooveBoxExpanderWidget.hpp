struct GrooveBoxExpanderWidget : ModuleWidget
{
  float mute_button_positions[NUMBER_OF_TRACKS][2] = {
    {10.16, 10},
    {10.16, 20},
    {10.16, 30},
    {10.16, 40},
    {10.16, 50},
    {10.16, 60},
    {10.16, 70},
    {10.16, 80}
  };

  float solo_button_positions[NUMBER_OF_TRACKS][2] = {
    {25.4, 10},
    {25.4, 20},
    {25.4, 30},
    {25.4, 40},
    {25.4, 50},
    {25.4, 60},
    {25.4, 70},
    {25.4, 80}
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
    }
  }

};
