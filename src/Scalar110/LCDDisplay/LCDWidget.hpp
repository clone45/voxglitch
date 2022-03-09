#include "Display.hpp"
#include "DisplayParams.hpp"
#include "DisplaySampleSelect.hpp"

struct LCDWidget : TransparentWidget
{
  Scalar110 *module;
  DisplayParams display_params;
  DisplaySampleSelect display_sample_select;

  LCDWidget(Scalar110 *module)
  {
    this->module = module;
    display_params.module = module;
    box.size = Vec(LCD_DISPLAY_WIDTH, LCD_DISPLAY_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    if(module)
    {
      switch(module->lcd_function)
      {
        case LCD_VALUES_DISPLAY:
          display_params.draw(args.vg);
          break;
        case LCD_SAMPLES_DISPLAY:
          display_sample_select.draw(args.vg);
          break;
        /*
        case LCD_ENGINE_DISPLAY:
          display_params.draw(args.vg);
          break;

        */
      }
    }

  }
};
