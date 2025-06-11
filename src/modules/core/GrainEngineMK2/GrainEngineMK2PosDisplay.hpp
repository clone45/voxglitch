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

      // unsigned int x = module->draw_position * (DRAW_AREA_WIDTH - 2);
      // ^ this is incorrect.  Instead, if x >= DRAW_AREA_WIDTH, x = DRAW_AREA_WIDTH - 1

      // TODO: multiply x instead of adjusting it
      float x = (module->draw_position * DRAW_AREA_WIDTH) * .982;
      // if (x >= DRAW_AREA_WIDTH) x = DRAW_AREA_WIDTH - 4;

      // Grey background
      nvgBeginPath(vg);
      nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
      nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
      nvgFill(vg);

      // overlay
      nvgBeginPath(vg);
      nvgRect(vg, 2, 2, x, DRAW_AREA_HEIGHT - 4);
      nvgFillColor(vg, nvgRGBA(255, 255, 255, 170));
      nvgFill(vg);
    }
    // Paint static content for library display
    else
    {

    }

    nvgRestore(vg);
  }

};
