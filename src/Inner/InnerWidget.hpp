struct InnerWidget : VoxglitchModuleWidget
{
  InnerWidget(Inner *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/inner_front_panel.svg")));

    // Inputs

    // Column #1
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.94, 20.702)), module, Inner::INPUT_1));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.94, 40.702)), module, Inner::INPUT_2));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.94, 60.702)), module, Inner::INPUT_3));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.94, 80.702)), module, Inner::INPUT_4));

    // Column #2
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 20.702)), module, Inner::INPUT_5));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 40.702)), module, Inner::INPUT_6));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 60.702)), module, Inner::INPUT_7));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.94, 80.702)), module, Inner::INPUT_8));

    // Outputs

    // Column #1
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.94, 20.702)), module, Inner::OUTPUT_1));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.94, 40.702)), module, Inner::OUTPUT_2));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.94, 60.702)), module, Inner::OUTPUT_3));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.94, 80.702)), module, Inner::OUTPUT_4));

    // Column #2
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94, 20.702)), module, Inner::OUTPUT_5));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94, 40.702)), module, Inner::OUTPUT_6));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94, 60.702)), module, Inner::OUTPUT_7));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94, 80.702)), module, Inner::OUTPUT_8));

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

