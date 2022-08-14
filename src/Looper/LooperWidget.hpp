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

    // Load and apply theme
    theme.load("looper");
    applyTheme();

    // Add output jacks
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_LEFT"), module, Looper::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_RIGHT"), module, Looper::AUDIO_OUTPUT_RIGHT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, Looper::RESET_INPUT));

    // Add volume slider
    //
    // Here's code for a longer slider:
    // https://github.com/algoritmarte/AlgoritmarteVCVPlugin/blob/main/src/Zefiro.hpp
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("VOLUME_SLIDER"), module, Looper::VOLUME_SLIDER));
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
