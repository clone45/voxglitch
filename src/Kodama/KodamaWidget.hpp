struct KodamaWidget : VoxglitchModuleWidget
{
  KodamaWidget(Kodama *module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("kodama");
    applyTheme();

    addInput(createInputCentered<VoxglitchInputPort>(Vec(45.0, 63.3706), module, Kodama::STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(45.0, 120.9109), module, Kodama::RESET_INPUT));

    addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 217.3509), module, Kodama::PREV_SEQUENCE_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 217.3509), module, Kodama::NEXT_SEQUENCE_INPUT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(24.0, 245.0), module, Kodama::PREV_BUTTON_PARAM));    
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(64.0, 245.0), module, Kodama::NEXT_BUTTON_PARAM));

    // addInput(createInputCentered<VoxglitchInputPort>(Vec(30.0, 240.0), module, Kodama::CV_SEQUENCE_SELECT));

    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(65.5748, 336.6334), module, Kodama::GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(25.6417, 336.6334), module, Kodama::EOL_OUTPUT)); // end of sequence output

    // Add display
    KodamaReadoutWidget *kodama_readout_widget = new KodamaReadoutWidget();
    kodama_readout_widget->box.pos = Vec(16.6063, 170.5335);
    kodama_readout_widget->module = module;
    addChild(kodama_readout_widget);

  }

  struct LoadFileMenuItem : MenuItem
  {
    Kodama *module;

    void onAction(const event::Action &e) override
    {
      std::string path = module->selectFileVCV();
      module->loadData(path);
      module->path = path;
    }
  };

  void appendContextMenu(Menu *menu) override
  {
    Kodama *module = dynamic_cast<Kodama *>(this->module);
    assert(module);

    // Menu in development
    menu->addChild(new MenuEntry); // For spacing only

    LoadFileMenuItem *load_file_menu_item = createMenuItem<LoadFileMenuItem>("Load File");
    load_file_menu_item->module = module;
    menu->addChild(load_file_menu_item);
  }

};
