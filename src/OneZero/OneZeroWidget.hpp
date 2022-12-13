struct OneZeroWidget : VoxglitchModuleWidget
{
  OneZeroWidget(OneZero *module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("onezero");
    applyTheme();

    addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 63.3706), module, OneZero::STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 63.3706), module, OneZero::RESET_INPUT));

    addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 157.3509), module, OneZero::PREV_SEQUENCE_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 157.3509), module, OneZero::NEXT_SEQUENCE_INPUT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(24.0, 157.3509 + 28.4537), module, OneZero::PREV_BUTTON_PARAM));    
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(64.0, 157.3509 + 28.4537), module, OneZero::NEXT_BUTTON_PARAM));

    addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 233.47), module, OneZero::ZERO_SEQUENCE_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 233.47), module, OneZero::CV_SEQUENCE_SELECT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(Vec(24.0, 233.47 + 28.4537), module, OneZero::ZERO_BUTTON_PARAM));    
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(65.5748, 233.47 + 28.4537), module, OneZero::CV_SEQUENCE_ATTN_KNOB));    
 

    // addInput(createInputCentered<VoxglitchInputPort>(Vec(30.0, 240.0), module, OneZero::CV_SEQUENCE_SELECT));

    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(65.5748, 336.6334), module, OneZero::GATE_OUTPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(25.6417, 336.6334), module, OneZero::EOL_OUTPUT)); // end of sequence output

    // Add display
    OneZeroReadoutWidget *one_zero_readout_widget = new OneZeroReadoutWidget();
    one_zero_readout_widget->box.pos = Vec(16.6063, 110.5335);
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

  /*
  struct OneShotMode : MenuItem {
    OneZero *module;

    void onAction(const event::Action &e) override {
      module->one_shot_mode ^= true; // flip the value
    }
  };
  */

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
     /*
    menu->addChild(new MenuSeparator());

    // One Shot Mode

    OneShotMode *one_shot_mode = createMenuItem<OneShotMode>("One-Shot Mode", CHECKMARK(module->one_shot_mode));
    one_shot_mode->module = module;
    menu->addChild(one_shot_mode);
    */
  }

};
