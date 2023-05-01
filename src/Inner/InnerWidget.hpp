struct InnerWidget : VoxglitchModuleWidget
{
  InnerWidget(Inner *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/inner_front_panel.svg")));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 20.702)), module, Inner::PARAM1_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 40.702)), module, Inner::PARAM2_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 60.702)), module, Inner::PARAM3_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 80.702)), module, Inner::PARAM4_CV_INPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.94, 20.702)), module, Inner::PARAM5_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.94, 40.702)), module, Inner::PARAM6_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.94, 60.702)), module, Inner::PARAM7_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.94, 80.702)), module, Inner::PARAM8_CV_INPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.94, 20.702)), module, Inner::PITCH_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.94, 40.702)), module, Inner::GATE_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.94, 114.702)), module, Inner::AUDIO_OUTPUT));
  }

  struct LoadFileMenuItem : MenuItem
  {
    Inner *module;

    void onAction(const event::Action &e) override
    {
      std::string path = module->selectFileVCV();
      module->load_config_file(path);
    }
  };

  void appendContextMenu(Menu *menu) override
  {
    Inner *module = dynamic_cast<Inner *>(this->module);
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

