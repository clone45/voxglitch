struct GrainEngineMK2ExpanderWidget : ModuleWidget
{
  GrainEngineMK2ExpanderWidget(GrainEngineMK2Expander* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_mk2_expander_front_panel.svg")));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 50)), module, GrainEngineMK2Expander::RECORD_START_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 60)), module, GrainEngineMK2Expander::RECORD_STOP_INPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 76)), module, GrainEngineMK2Expander::SAMPLE_SLOT_INPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 30)), module, GrainEngineMK2Expander::AUDIO_IN_LEFT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 30)), module, GrainEngineMK2Expander::AUDIO_IN_RIGHT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20, 100)), module, GrainEngineMK2Expander::PASSTHROUGH_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30, 100)), module, GrainEngineMK2Expander::PASSTHROUGH_RIGHT));
  }

  void appendContextMenu(Menu *menu) override
  {
    GrainEngineMK2Expander *module = dynamic_cast<GrainEngineMK2Expander*>(this->module);
    assert(module);
  }
};
