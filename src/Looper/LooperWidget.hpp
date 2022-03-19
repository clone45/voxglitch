struct LooperWidget : ModuleWidget
{
  LooperWidget(Looper* module)
  {
    setModule(module);

    // Set the background SVG panel.  This should be blank
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/looper/looper_front_panel.svg")));

    // Load up the background PNG and add it to the panel
    PNGPanel *png_panel = new PNGPanel("res/looper/looper_3he_baseplate.png", 5.08 * 3, 128.5);
    addChild(png_panel);

    // Add typography layer
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/looper/looper_typography_t2.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);

    // Add output jacks
    addOutput(createOutputCentered<VoxglitchOutputPort>(mm2px(Vec(7.58, 103)), module, Looper::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(mm2px(Vec(7.58, 115.2)), module, Looper::AUDIO_OUTPUT_RIGHT));

    addInput(createInputCentered<VoxglitchInputPort>(mm2px(Vec(7.55, 25.5)), module, Looper::RESET_INPUT));

    // Add waveform display (see LooperWaveformDisplay.hpp)
    LooperWaveformDisplay *looper_waveform_display = new LooperWaveformDisplay();
    looper_waveform_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    looper_waveform_display->module = module;
    addChild(looper_waveform_display);

    // Add custom switch
    // addParam(createParamCentered<roundToggle>(mm2px(Vec(7.55, 11.54)), module, Looper::SWITCH_TEST));
    addParam(createParamCentered<VoxglitchRoundToggleLampSwitch>(Vec(22.393309,33.974804), module, Looper::SWITCH_TEST));
  }

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
  }
};
