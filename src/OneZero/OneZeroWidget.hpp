struct OneZeroWidget : VoxglitchModuleWidget
{
  OneZeroWidget(OneZero *module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("onezero");
    applyTheme();

    addInput(createInputCentered<VoxglitchInputPort>(themePos("STEP_INPUT"), module, OneZero::STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, OneZero::RESET_INPUT));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("PREV_INPUT"), module, OneZero::PREV_SEQUENCE_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("NEXT_INPUT"), module, OneZero::NEXT_SEQUENCE_INPUT));
    
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("PREV_BUTTON"), module, OneZero::PREV_BUTTON_PARAM));    
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("NEXT_BUTTON"), module, OneZero::NEXT_BUTTON_PARAM));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("ZERO_SEQUENCE_INPUT"), module, OneZero::ZERO_SEQUENCE_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("CV_SEQUENCE_SELECT"), module, OneZero::CV_SEQUENCE_SELECT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("ZERO_BUTTON_PARAM"), module, OneZero::ZERO_BUTTON_PARAM));    
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("CV_SEQUENCE_ATTN_KNOB"), module, OneZero::CV_SEQUENCE_ATTN_KNOB));    
 

    // addInput(createInputCentered<VoxglitchInputPort>(Vec(30.0, 240.0), module, OneZero::CV_SEQUENCE_SELECT));

    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUT"), module, OneZero::GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("EOL_OUTPUT"), module, OneZero::EOL_OUTPUT)); // end of sequence output

    // Add display
    OneZeroReadoutWidget *one_zero_readout_widget = new OneZeroReadoutWidget();
    one_zero_readout_widget->box.pos = themePos("READOUT");
    one_zero_readout_widget->module = module;
    addChild(one_zero_readout_widget);

  }

  struct LoadFileMenuItem : MenuItem
  {
    OneZero *module;

    void onAction(const event::Action &e) override
    {
      std::string path = module->selectFileVCV();
      module->loadData(path);
      module->path = path;
    }
  };

  void appendContextMenu(Menu *menu) override
  {
    OneZero *module = dynamic_cast<OneZero *>(this->module);
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
