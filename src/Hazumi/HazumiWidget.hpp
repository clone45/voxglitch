struct HazumiWidget : ModuleWidget
{
  HazumiWidget(Hazumi* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hazumi_front_panel.svg")));

    // Step & Reset inputs
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_1 AND_A_HALF_ROW)), module, Hazumi::STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_3 AND_THREE_QUARTERS_ROW - 0.5)), module, Hazumi::RESET_INPUT));

    // Outputs
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_6)), module, Hazumi::GATE_OUTPUT_1));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_7 AND_A_QUARTER_ROW)), module, Hazumi::GATE_OUTPUT_2));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_8 AND_A_HALF_ROW)), module, Hazumi::GATE_OUTPUT_3));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_9 AND_THREE_QUARTERS_ROW)), module, Hazumi::GATE_OUTPUT_4));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_11)), module, Hazumi::GATE_OUTPUT_5));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_12 AND_A_QUARTER_ROW)), module, Hazumi::GATE_OUTPUT_6));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_13 AND_A_HALF_ROW)), module, Hazumi::GATE_OUTPUT_7));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_14 AND_THREE_QUARTERS_ROW)), module, Hazumi::GATE_OUTPUT_8));

    HazumiSequencerDisplay *hazumi_sequencer_display = new HazumiSequencerDisplay();
    hazumi_sequencer_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    hazumi_sequencer_display->module = module;
    addChild(hazumi_sequencer_display);
  }

};
