struct OnePointWidget : VoxglitchModuleWidget
{
  OnePointWidget(OnePoint *module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("onepoint");
    applyTheme();

    addInput(createInputCentered<VoxglitchInputPort>(themePos("STEP_INPUT"), module, OnePoint::STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, OnePoint::RESET_INPUT));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("PREV_INPUT"), module, OnePoint::PREV_SEQUENCE_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("NEXT_INPUT"), module, OnePoint::NEXT_SEQUENCE_INPUT));
    
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("PREV_BUTTON"), module, OnePoint::PREV_BUTTON_PARAM));    
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("NEXT_BUTTON"), module, OnePoint::NEXT_BUTTON_PARAM));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("ZERO_SEQUENCE_INPUT"), module, OnePoint::ZERO_SEQUENCE_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("CV_SEQUENCE_SELECT"), module, OnePoint::CV_SEQUENCE_SELECT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("ZERO_BUTTON_PARAM"), module, OnePoint::ZERO_BUTTON_PARAM));    
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("CV_SEQUENCE_ATTN_KNOB"), module, OnePoint::CV_SEQUENCE_ATTN_KNOB));    
 
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("CV_OUTPUT"), module, OnePoint::CV_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("EOL_OUTPUT"), module, OnePoint::EOL_OUTPUT)); // end of sequence output

    // Add display
    OnePointReadoutWidget *one_point_readout_widget = new OnePointReadoutWidget();
    one_point_readout_widget->box.pos = themePos("READOUT");
    one_point_readout_widget->module = module;
    addChild(one_point_readout_widget);

  }

  struct LoadFileMenuItem : MenuItem
  {
    OnePoint *module;

    void onAction(const event::Action &e) override
    {
      std::string path = module->selectFileVCV();
      module->loadData(path);
      module->path = path;
    }
  };

  void appendContextMenu(Menu *menu) override
  {
    OnePoint *module = dynamic_cast<OnePoint *>(this->module);
    assert(module);

    // Menu in development
    menu->addChild(new MenuEntry); // For spacing only

    LoadFileMenuItem *load_file_menu_item = createMenuItem<LoadFileMenuItem>("Load File");
    load_file_menu_item->module = module;
    menu->addChild(load_file_menu_item);

    if(module->path != "")
    {
      std::string filename = rack::system::getFilename(module->path);
      menu->addChild(createMenuLabel(filename));
    }
    else
    {
      menu->addChild(createMenuLabel("No file loaded"));
    }
  }

};
