struct KodamaWidget : VoxglitchModuleWidget
{
  KodamaWidget(Kodama* module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("kodama");
    applyTheme();

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
