struct VoxglitchSliderLong : app::SvgSlider {
	VoxglitchSliderLong() {
		maxHandlePos = mm2px(Vec(-.375, 12.0));
		minHandlePos = mm2px(Vec(-.375, 52));
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/VoxglitchSliderLong.svg")));
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/VoxglitchSliderHandle.svg")));
	}
};

struct LooperWidget : VoxglitchSamplerModuleWidget
{
  LooperWidget(Looper* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/looper_front_panel.svg")));

    // Add output jacks
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.560, 100.39)), module, Looper::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.560, 112.4375)), module, Looper::AUDIO_OUTPUT_RIGHT));

    // Add reset input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.560, 24.09375)), module, Looper::RESET_INPUT));

    // Add waveform display (see LooperWaveformDisplay.hpp)
    /*
    LooperWaveformDisplay *looper_waveform_display = new LooperWaveformDisplay();
    looper_waveform_display->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    looper_waveform_display->box.size.x = DRAW_AREA_WIDTH;
    looper_waveform_display->box.size.y = DRAW_AREA_HEIGHT;
    looper_waveform_display->module = module;
    addChild(looper_waveform_display);
    */
    // Add volume slider
    //
    // Here's code for a longer slider:
    // https://github.com/algoritmarte/AlgoritmarteVCVPlugin/blob/main/src/Zefiro.hpp
    addParam(createParamCentered<VoxglitchSliderLong>(mm2px(Vec(7.560, 60)), module, Looper::VOLUME_SLIDER));

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

    SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
    sample_interpolation_menu_item->module = module;
    menu->addChild(sample_interpolation_menu_item);
  }
};
