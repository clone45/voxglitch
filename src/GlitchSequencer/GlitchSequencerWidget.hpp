struct GlitchSequencerWidget : ModuleWidget
{
  GlitchSequencerWidget(GlitchSequencer* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/glitch_sequencer_front_panel.svg")));

    float button_spacing = 9.8; // 9.1
    float button_group_x = 53.0;
    float button_group_y = 109.0;

    float inputs_y = 116.0;

    // Step
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, inputs_y)), module, GlitchSequencer::STEP_INPUT));

    // Reset
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 13.544, inputs_y)), module, GlitchSequencer::RESET_INPUT));

    // Length
    addParam(createParamCentered<Trimpot>(mm2px(Vec(10 + (13.544 * 2), inputs_y)), module, GlitchSequencer::LENGTH_KNOB));

    for(unsigned int i=0; i<NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      // Trigger group buttons and lights
      addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_LIGHTS + i));

      // Add outputs
      addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y + 10)), module, GlitchSequencer::GATE_OUTPUTS + i));
    }

    CellularAutomatonDisplay *ca_display = new CellularAutomatonDisplay();
    ca_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    ca_display->module = module;
    addChild(ca_display);

    LengthReadoutDisplay *length_readout_display = new LengthReadoutDisplay();
    length_readout_display->box.pos = mm2px(Vec(30.0, 122.0));
    length_readout_display->module = module;
    addChild(length_readout_display);
  }

  void appendContextMenu(Menu *menu) override
  {
    GlitchSequencer *module = dynamic_cast<GlitchSequencer*>(this->module);
    assert(module);
  }

};
