struct GlitchSequencerWidget : ModuleWidget
{
  GlitchSequencerWidget(GlitchSequencer* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/glitch_sequencer_front_panel.svg")));

    float button_spacing = 9.6; // 9.1
    float button_group_x = 53.0;
    float button_group_y = 109.0;

    float inputs_y = 116.0;

    // Step
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, inputs_y)), module, GlitchSequencer::STEP_INPUT));

    // Reset
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10 + 13.544, inputs_y)), module, GlitchSequencer::RESET_INPUT));

    // Length
    addParam(createParamCentered<Trimpot>(mm2px(Vec(10 + (13.544 * 2), inputs_y)), module, GlitchSequencer::LENGTH_KNOB));

    // Sequence 1 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x, button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_1_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x, button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_1_LIGHT));
    // Sequence 2 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_2_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 1.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_2_LIGHT));
    // Sequence 3 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_3_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 2.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_3_LIGHT));
    // Sequence 4 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_4_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 3.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_4_LIGHT));
    // Sequence 5 button
    addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_5_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * 4.0), button_group_y)), module, GlitchSequencer::TRIGGER_GROUP_5_LIGHT));

    float y = button_group_y + 10;
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x, y)), module, GlitchSequencer::GATE_OUTPUT_1));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 1.0), y)), module, GlitchSequencer::GATE_OUTPUT_2));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 2.0), y)), module, GlitchSequencer::GATE_OUTPUT_3));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 3.0), y)), module, GlitchSequencer::GATE_OUTPUT_4));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(button_group_x + (button_spacing * 4.0), y)), module, GlitchSequencer::GATE_OUTPUT_5));

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
