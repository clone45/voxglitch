/*

Probably the weirdest thing about this code is the absense of a DPSliderDisplay
"x" position.  The x position relative to the containing box is always 0.0.
Changing the x position of a slider on the panel is done in the widget, like:

float x = 50.0;
float y = 10.0;

DPSliderDisplay *dp_slider_display = new DPSliderDisplay(&module->sliders[0]);
dp_slider_display->setPosition(mm2px(Vec(x, y)));   <====== HERE
dp_slider_display->setSize(Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT));
dp_slider_display->module = module;
addChild(dp_slider_display);

*/

struct DPSliderDisplay : TransparentWidget
{
  DigitalProgrammer *module;
  double max_bar_height = 190;
  Vec drag_position;
  unsigned int column = 0;

  DPSliderDisplay(unsigned int column)
  {
    this->column = column;
  }

  void drawLayer(const DrawArgs& args, int layer) override
  {
  	if (layer == 1)
    {
      const auto vg = args.vg;

      // Save the drawing context to restore later
      nvgSave(vg);

      if(module)
      {
        // For testing, draw rect showing draw area
        // -----------------------------------------
        /*
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, SLIDER_HEIGHT);
        nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
        nvgFill(vg);
        */
        // -----------------------------------------

        double value = module->sliders[module->selected_bank][column].getValue();

        drawSliderBackground(vg, nvgRGBA(53, 64, 85, 255));
        drawSlider(vg, value, nvgRGBA(156, 167, 185, 255));
      }
      nvgRestore(vg);
    }
  }

  void drawBlueOverlay(NVGcontext *vg, double width, double height)
  {
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, width, height);
    nvgFillColor(vg, nvgRGBA(0, 100, 255, 28));
    nvgFill(vg);
  }


  void drawSliderBackground(NVGcontext *vg, NVGcolor color)
  {
    // double x = 0;
    double y = SLIDER_HEIGHT;

    drawBar(vg, y, color);
  }


  void drawSlider(NVGcontext *vg, double value, NVGcolor color)
  {
    // Convert "value", which ranges from 0.0 to 1.0, to the high and low
    // constraints of the slider.
    double y = value * SLIDER_HEIGHT;

    drawBar(vg, y, color);
  }


  void drawBar(NVGcontext *vg, double y, NVGcolor color)
  {
    nvgBeginPath(vg);
    nvgRect(vg, 0.0, SLIDER_HEIGHT - y, SLIDER_WIDTH, y);
    nvgFillColor(vg, color);
    nvgFill(vg);
  }

  //
  // void editBar(Vec mouse_position)
  //
  // Called when the user clicks to edit one of the sequencer values.  Sets
  // the sequencer value that the user has selected, then sets some variables
  // for drawing the tooltip in this struct's draw(..) method.
  //

  void editBar(Vec mouse_position)
  {
    // Note: This only works when the slider height is equal to the draw area
    // height.  If it didn't there'd be more thinking to do.  Luckily it is.
    double new_value = (SLIDER_HEIGHT - mouse_position.y) / SLIDER_HEIGHT;
    if (new_value < 0) new_value = 0;
    if (new_value > 1) new_value = 1;

    // Tooltip drawing is done in the draw method
    /*
    draw_tooltip = true;
    draw_tooltip_index = clicked_bar_x_index;
    draw_tooltip_y = clicked_y;
    tooltip_value = module->selected_voltage_sequencer->getOutput(clicked_bar_x_index);
    */
    this->module->sliders[module->selected_bank][column].setValue(new_value);
  }

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      drag_position = e.pos;
      this->editBar(e.pos);
    }
  }

  void onDragMove(const event::DragMove &e) override
  {
    TransparentWidget::onDragMove(e);
    float zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));
    editBar(drag_position);
  }

};
