//
// UpdatesWidget
//
// This widget shows up to display recent changes to the GrooveBox to the
// end-user.

struct UpdatesWidget : TransparentWidget
{
  GrooveBox *module;

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    nvgSave(vg);
    nvgBeginPath(vg);

    if(module)
    {
      float width = 400;
      float height = 200;

      float x = (MODULE_WIDTH - width) / 2.0;
      float y = (MODULE_HEIGHT - height) / 2.0;

      nvgRoundedRect(vg, x, y, width, height, 5);
      nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
      nvgFill(vg);
    }

    nvgRestore(vg);
  }

};
