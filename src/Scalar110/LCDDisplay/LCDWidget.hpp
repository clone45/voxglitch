#include "Display.hpp"
#include "DisplayParams.hpp"

struct LCDWidget : TransparentWidget
{
  Scalar110 *module;
  DisplayParams display_params;
  // DisplayEngine display_engine;
  // DisplaySample display_sample;

  LCDWidget(Scalar110 *module)
  {
    this->module = module;
    display_params.module = module;
    box.size = Vec(LCD_DISPLAY_WIDTH, LCD_DISPLAY_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    display_params.draw(args.vg);
  }
};
