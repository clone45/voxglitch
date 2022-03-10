#include "Display.hpp"
#include "DisplayParams.hpp"
#include "DisplaySampleSelect.hpp"

struct LCDWidget : TransparentWidget
{
  Scalar110 *module;
  DisplayParams display_params;
  DisplaySampleSelect display_sample_select;
  Display *active_display = &display_params;
  Vec drag_position;

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
          active_display = &display_params;
          // display_params.draw(args.vg);
          break;
        case LCD_SAMPLES_DISPLAY:
          active_display = &display_sample_select;
          // display_sample_select.draw(args.vg);
          break;
        /*
        case LCD_ENGINE_DISPLAY:
          display_params.draw(args.vg);
          break;

        */
      }





      // I think that this is crashing it
      // This is crashing IF the engine is set to sample display
      active_display->draw(args.vg);
    }

  }

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      drag_position = e.pos;

      active_display->onButton(e.pos);

      // this->editBar(e.pos);
    }
  }

  void onDragMove(const event::DragMove &e) override
  {
    TransparentWidget::onDragMove(e);

    float zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));
    active_display->onDragMove(drag_position);
  }

};
