struct HazumiWidget : ModuleWidget
{
  HazumiWidget(Hazumi* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/hazumi_front_panel.svg")));

    // Step
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 0.0)), module, Hazumi::STEP_INPUT));

    HazumiSequencerDisplay *hazumi_sequencer_display = new HazumiSequencerDisplay();
    hazumi_sequencer_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    hazumi_sequencer_display->module = module;
    addChild(hazumi_sequencer_display);
  }

};
