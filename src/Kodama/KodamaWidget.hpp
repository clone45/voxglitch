struct VoxglitchSliderLong : app::SvgSlider {
	VoxglitchSliderLong() {
		maxHandlePos = mm2px(Vec(-.10, -3.0));
		minHandlePos = mm2px(Vec(-.10, 36.0));
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/VoxglitchSliderLong.svg")));
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/VoxglitchSliderHandle.svg")));
	}
};

struct KodamaWidget : VoxglitchSamplerModuleWidget
{
  KodamaWidget(Kodama* module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("kodama");
    applyTheme();

    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_1"), module, Kodama::SLIDERS + 0));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_2"), module, Kodama::SLIDERS + 1));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_3"), module, Kodama::SLIDERS + 2));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_4"), module, Kodama::SLIDERS + 3));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_5"), module, Kodama::SLIDERS + 4));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_6"), module, Kodama::SLIDERS + 5));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_7"), module, Kodama::SLIDERS + 6));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_8"), module, Kodama::SLIDERS + 7));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_9"), module, Kodama::SLIDERS + 8));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_10"), module, Kodama::SLIDERS + 9));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_11"), module, Kodama::SLIDERS + 10));
    addParam(createParamCentered<VoxglitchSliderLong>(themePos("SLIDERS_12"), module, Kodama::SLIDERS + 11));


    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.216 + 0, 114.702)), module, Kodama::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.94 + 0, 114.702)), module, Kodama::AUDIO_OUTPUT_RIGHT));

    // =================== PLACE COMPONENTS ====================================
  }

  void appendContextMenu(Menu *menu) override
  {
    Kodama *module = dynamic_cast<Kodama*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    // menu->addChild(createMenuLabel("Load Sample"));

    KodamaLoadSample *kodama_load_sample = new KodamaLoadSample();
    kodama_load_sample->module = module;
    kodama_load_sample->text = "Load sample";
    menu->addChild(kodama_load_sample);

    // Interpolation menu
    menu->addChild(new MenuEntry); // For spacing only
    SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
    sample_interpolation_menu_item->module = module;
    menu->addChild(sample_interpolation_menu_item);
  }

};
