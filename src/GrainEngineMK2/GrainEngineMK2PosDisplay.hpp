struct GrainEngineMK2PosDisplay : TransparentWidget
{
  GrainEngineMK2 *module;

  GrainEngineMK2PosDisplay()
  {
    // box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    if(module)
    {
      if(module->draw_position < 0) module->draw_position = 0;
      if(module->draw_position > 1) module->draw_position = 1;

      unsigned int x = module->draw_position * (DRAW_AREA_WIDTH - 2);

      // Grey background
      nvgBeginPath(vg);
      nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
      nvgFillColor(vg, nvgRGB(55, 55, 55));
      nvgFill(vg);

      // Red overlay
      nvgBeginPath(vg);
      nvgRect(vg, 1, 1, x, DRAW_AREA_HEIGHT - 2);
      nvgFillColor(vg, nvgRGB(255, 255, 255));
      nvgFill(vg);
    }
    // Paint static content for library display
    else
    {

    }

    nvgRestore(vg);
  }

};
