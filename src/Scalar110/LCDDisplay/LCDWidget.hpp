#include "LCDPage.hpp"
#include "LCDPageParameterValues.hpp"
#include "LCDPageSamples.hpp"
#include "LCDPageEngine.hpp"

struct LCDWidget : TransparentWidget
{
  Scalar110 *module;

  LCDPageEngine lcd_page_engine;
  LCDPageParameterValues lcd_page_parameter_values;
  LCDPageSamples lcd_page_samples;

  unsigned int active_page_index = 0;
  LCDPage *lcd_pages[NUMBER_OF_LCD_PAGES];

  Vec drag_position;

  LCDWidget(Scalar110 *module)
  {
    this->module = module;

    // Don't forget to update this when adding pages!
    lcd_pages[LCD_PAGE_ENGINE] = &lcd_page_engine;
    lcd_pages[LCD_PAGE_PARAMETER_VALUES] = &lcd_page_parameter_values;
    lcd_pages[LCD_PAGE_SAMPLES] = &lcd_page_samples;

    for(unsigned int lcd_page_index = 0; lcd_page_index < NUMBER_OF_LCD_PAGES; lcd_page_index++)
    {
      lcd_pages[lcd_page_index]->module = module;
    }
    // I only set the lcd_page_params module pointer, not the others
    // How the other modules ever got a pointer to module is unclear

    // lcd_page_parameter_values.module = module; // HERE'S WHERE THE PROBLEM IS

    box.size = Vec(LCD_DISPLAY_WIDTH, LCD_DISPLAY_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    if(module)
    {
      active_page_index = module->lcd_page_index;

      // I think that this is crashing it
      // This is crashing IF the engine is set to sample display
      lcd_pages[active_page_index]->draw(args.vg);
    }

  }

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      drag_position = e.pos;

      lcd_pages[active_page_index]->onButton(e.pos);

      // this->editBar(e.pos);
    }
  }

  void onDragMove(const event::DragMove &e) override
  {
    TransparentWidget::onDragMove(e);

    float zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));
    lcd_pages[active_page_index]->onDragMove(drag_position);
  }

  void step() override {
    TransparentWidget::step();
  }

};
