struct GrooveBoxExpanderWidget : ModuleWidget
{
  // 1st columen 20

  float volume_knob_positions[NUMBER_OF_TRACKS][2] = {
    {25.411 + 6.0, 30 * 2.952756},
    {25.411 + 6.0, 40 * 2.952756},
    {25.411 + 6.0, 50 * 2.952756},
    {25.411 + 6.0, 60 * 2.952756},
    {25.411 + 6.0, 70 * 2.952756},
    {25.411 + 6.0, 80 * 2.952756},
    {25.411 + 6.0, 90 * 2.952756},
    {25.411 + 6.0, 100 * 2.952756}
  };

  float mute_button_positions[NUMBER_OF_TRACKS][2] = {
    {72.167 + 6.0, 30 * 2.952756},
    {72.167 + 6.0, 40 * 2.952756},
    {72.167 + 6.0, 50 * 2.952756},
    {72.167 + 6.0, 60 * 2.952756},
    {72.167 + 6.0, 70 * 2.952756},
    {72.167 + 6.0, 80 * 2.952756},
    {72.167 + 6.0, 90 * 2.952756},
    {72.167 + 6.0, 100 * 2.952756}
  };

  float solo_button_positions[NUMBER_OF_TRACKS][2] = {
    {145.947 + 6.0, 30 * 2.952756},
    {145.947 + 6.0, 40 * 2.952756},
    {145.947 + 6.0, 50 * 2.952756},
    {145.947 + 6.0, 60 * 2.952756},
    {145.947 + 6.0, 70 * 2.952756},
    {145.947 + 6.0, 80 * 2.952756},
    {145.947 + 6.0, 90 * 2.952756},
    {145.947 + 6.0, 100 * 2.952756}
  };

  float gate_output_positions[NUMBER_OF_TRACKS][2] = {
    {232.617, 30 * 2.952756},
    {232.617, 40 * 2.952756},
    {232.617, 50 * 2.952756},
    {232.617, 60 * 2.952756},
    {232.617, 70 * 2.952756},
    {232.617, 80 * 2.952756},
    {232.617, 90 * 2.952756},
    {232.617, 100 * 2.952756}
  };

  struct ModdedCL1362 : SvgPort {
  	ModdedCL1362() {
  		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/modded_CL1362.svg")));
  	}
  };

  GrooveBoxExpanderWidget(GrooveBoxExpander* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groove_box_expander_front_panel.svg")));

    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      //
      // Add volume buttons and inputs
      //
      float volume_knob_x = volume_knob_positions[i][0];
      float volume_knob_y = volume_knob_positions[i][1];
      addParam(createParamCentered<Trimpot>(Vec(volume_knob_x, volume_knob_y), module, GrooveBoxExpander::VOLUME_KNOBS + i));

      //
      // Add mute buttons and inputs
      //
      float mute_button_x = mute_button_positions[i][0];
      float mute_button_y = mute_button_positions[i][1];

      addParam(createLightParamCentered<VCVLightBezel<RedLight>>(Vec(mute_button_x, mute_button_y), module, GrooveBoxExpander::MUTE_BUTTONS + i, GrooveBoxExpander::MUTE_BUTTON_LIGHTS + i));
      addInput(createInputCentered<PJ301MPort>(Vec(mute_button_x + 29.52756, mute_button_y), module, GrooveBoxExpander::MUTE_INPUTS + i));

      //
      // Add solo buttons and inputs
      //

      float solo_button_x = solo_button_positions[i][0];
      float solo_button_y = solo_button_positions[i][1];

      addParam(createLightParamCentered<VCVLightBezel<GreenLight>>(Vec(solo_button_x, solo_button_y), module, GrooveBoxExpander::SOLO_BUTTONS + i, GrooveBoxExpander::SOLO_BUTTON_LIGHTS + i));
      addInput(createInputCentered<PJ301MPort>(Vec(solo_button_x + 29.52756, solo_button_y), module, GrooveBoxExpander::SOLO_INPUTS + i));

      //
      // Add track trigger outputs
      //
      addOutput(createOutputCentered<ModdedCL1362>(Vec(gate_output_positions[i][0], gate_output_positions[i][1]), module, GrooveBoxExpander::TRIGGER_OUTPUTS + i));

    }
  }

};
