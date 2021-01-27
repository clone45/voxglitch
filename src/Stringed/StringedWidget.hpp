struct StringedWidget : ModuleWidget
{
  StringedWidget(Stringed* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/stringed_front_panel.svg")));

    // Outputs
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_6)), module, Stringed::GATE_OUTPUT_1));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_7 AND_A_QUARTER_ROW)), module, Stringed::GATE_OUTPUT_2));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(COLUMN_14, ROW_8 AND_A_HALF_ROW)), module, Stringed::GATE_OUTPUT_3));

    StringedDisplay *stringed_display = new StringedDisplay();
    stringed_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    stringed_display->module = module;
    addChild(stringed_display);
  }

};
