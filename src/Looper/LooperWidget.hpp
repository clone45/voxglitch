struct LooperWidget : ModuleWidget
{
  LooperWidget(Looper* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/looper_front_panel.svg")));

    // Add output jacks
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.560, ROW_12 AND_A_HALF_ROW)), module, Looper::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.560, ROW_14)), module, Looper::AUDIO_OUTPUT_RIGHT));

    // Add reset input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.560, ROW_3)), module, Looper::RESET_INPUT));

    // Add waveform display (see LooperWaveformDisplay.hpp)
    LooperWaveformDisplay *looper_waveform_display = new LooperWaveformDisplay();
    looper_waveform_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    looper_waveform_display->module = module;
    addChild(looper_waveform_display);
  }

  struct InterpolationOffOption : MenuItem {
    Looper *module;

    void onAction(const event::Action &e) override {
      module->interpolation = 0;
    }
  };

  struct InterpolationLinearOption : MenuItem {
    Looper *module;

    void onAction(const event::Action &e) override {
      module->interpolation = 1;
    }
  };

  struct SampleInterpolationMenuItem : MenuItem {
    Looper *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      InterpolationOffOption *interpolation_off_option = createMenuItem<InterpolationOffOption>("Off", CHECKMARK(module->interpolation == 0));
      interpolation_off_option->module = module;
      menu->addChild(interpolation_off_option);

      InterpolationLinearOption *interpolation_linear_option = createMenuItem<InterpolationLinearOption>("Linear", CHECKMARK(module->interpolation == 1));
      interpolation_linear_option->module = module;
      menu->addChild(interpolation_linear_option);

      return menu;
    }
  };

  void appendContextMenu(Menu *menu) override
  {
    Looper *module = dynamic_cast<Looper*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Sample"));

    // Add the sample slot to the right-click context menu
    LooperLoadSample *menu_item_load_sample = new LooperLoadSample();
    menu_item_load_sample->text = module->loaded_filename;
    menu_item_load_sample->module = module;
    menu->addChild(menu_item_load_sample);


    SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
    sample_interpolation_menu_item->module = module;
    menu->addChild(sample_interpolation_menu_item);
  }
};
