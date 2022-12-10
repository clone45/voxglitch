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

    addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 197.3489), module, Kodama::PREV_SEQUENCE_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 197.3489), module, Kodama::NEXT_SEQUENCE_INPUT));

    // addInput(createInputCentered<VoxglitchInputPort>(Vec(30.0, 240.0), module, Kodama::CV_SEQUENCE_SELECT));

    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(45.0, 336.6334), module, Kodama::GATE_OUTPUT));

  }

  struct LoadFileMenuItem : MenuItem
  {
    Kodama *module;

    void onAction(const event::Action &e) override
    {
      std::string path = module->selectFileVCV();
      module->loadData(path);
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
