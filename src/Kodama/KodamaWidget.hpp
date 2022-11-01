struct VoxglitchSliderLong : app::SvgSlider {
	VoxglitchSliderLong() {
		maxHandlePos = mm2px(Vec(-.10, -3.0));
		minHandlePos = mm2px(Vec(-.10, 36.0));
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/VoxglitchSliderLong.svg")));
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/VoxglitchSliderHandle.svg")));
	}
};

struct KodamaWidget : VoxglitchModuleWidget
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


    // Add rack screws
    if(theme.showScrews())
    {
      addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, 0)));
  		// addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  		addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  		// addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    // =================== PLACE COMPONENTS ====================================
  }

  void appendContextMenu(Menu *menu) override
  {
    Kodama *module = dynamic_cast<Kodama*>(this->module);
    assert(module);

    // add contect menu items here
  }

};
