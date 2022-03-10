struct CustomPanel : TransparentWidget
{
  void draw(const DrawArgs &args) override
  {
    std::shared_ptr<Image> img = APP->window->loadImage(asset::plugin(pluginInstance, "res/looper/looper_3he_baseplate.png"));

    int width, height;

    // Get the image size and store it in the width and height variables
    nvgImageSize(args.vg, img->handle, &width, &height);

    // Set the bounding box of the widget
    box.size = mm2px(Vec(5.08 * 3, 128.5));

    // Paint the .png background
    NVGpaint paint = nvgImagePattern(args.vg, 0.0, 0.0, box.size.x, box.size.y, 0.0, img->handle, 1.0);
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
    nvgFillPaint(args.vg, paint);
    nvgFill(args.vg);

    Widget::draw(args);
  }
};

struct roundToggle : app::SvgSwitch {
  roundToggle() {
    momentary = false;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/round_light_off.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/round_light5-4.svg")));
  }
};

  // Second layer
  /*
  std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/looper/looper_front_panel.svg"));
  SvgPanel *svgPanel = new SvgPanel;
  svgPanel->setBackground(svg);
  addChild(svgPanel);
  */



struct LooperWidget : ModuleWidget
{
  LooperWidget(Looper* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/looper/looper_front_panel.svg")));

    CustomPanel *custom_panel = new CustomPanel();
    addChild(custom_panel);

    // Add typography layer
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/looper/looper_typography_t2.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);

    // Add output jacks
    // addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.55, 103)), module, Looper::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<BlankPort>(mm2px(Vec(7.55, 103)), module, Looper::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<BlankPort>(mm2px(Vec(7.55, 115.2)), module, Looper::AUDIO_OUTPUT_RIGHT));

    // Add reset input
    addInput(createInputCentered<BlankPort>(mm2px(Vec(7.55, 25.5)), module, Looper::RESET_INPUT));

    // Add waveform display (see LooperWaveformDisplay.hpp)
    LooperWaveformDisplay *looper_waveform_display = new LooperWaveformDisplay();
    looper_waveform_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    looper_waveform_display->module = module;
    addChild(looper_waveform_display);

    // Add custom switch
    addParam(createParamCentered<roundToggle>(mm2px(Vec(6.72, 10.50)), module, Looper::SWITCH_TEST));
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
