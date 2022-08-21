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
    box.size = Vec(SLIDER_HEIGHT, SLIDER_HEIGHT);
  }

  void draw(const DrawArgs& args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    /* old, bright colors
    NVGcolor slider_background_color = nvgRGBA(31,39,42,255);
    NVGcolor slider_mouse_over_color = nvgRGBA(154,212,191,255);
    NVGcolor slider_background_mouse_over_color = nvgRGBA(52,60,63,255);
    NVGcolor slider_color = nvgRGBA(132,190,169,255);
    */

    // New colors to match Digital Sequencer
    NVGcolor slider_background_color = nvgRGBA(37,56,59,255);
    NVGcolor slider_mouse_over_color = nvgRGBA(223,234,236,255);
    NVGcolor slider_background_mouse_over_color = nvgRGBA(50,70,74,255);
    NVGcolor slider_color = nvgRGBA(149,163,165,255);


    if(module)
    {
      double value = module->sliders[module->selected_bank][column].getValue();

      if(module->is_moused_over_slider && (module->moused_over_slider == this->column))
      {
        // draw mouse-over background
        drawSliderBackground(vg, slider_background_mouse_over_color);

        NVGcolor fill_color = getSliderColor(slider_mouse_over_color);
        drawSlider(vg, value, fill_color);
        drawSlider(vg, value, nvgRGBA(255, 255, 255, 120));
      }
      else
      {
        // draw normal background
        drawSliderBackground(vg, slider_background_color); // was 53, 64, 85, 255

        NVGcolor fill_color = getSliderColor(slider_color);
        drawSlider(vg, value, fill_color);

        // Draw highlight for voltage additions

        if(module->visualize_sums && module->add_input_voltages[column] != 0)
        {
          float sum_value = value + (module->add_input_voltages[column] / 10.0);
          sum_value = clamp(sum_value, 0.0, 1.0);
          drawSlider(vg, sum_value, nvgRGBA(30, 30, 30, 60));
        }
      }

      // Draw label background
      /*
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0.0, 110, SLIDER_WIDTH, -110);
      nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 60));
      nvgFill(vg);
      */

      // Draw label, if any
      std::string to_display = module->labels[column];

      if(to_display != "")
      {
        nvgFontSize(args.vg, 14);
        nvgTextLetterSpacing(args.vg, 0);
        nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
        nvgRotate(args.vg, -M_PI / 2.0f);
        nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
        float x_position = 16;
        float y_position = -280;
        float wrap_at = 275.0; // Just throw your hands in the air!  And wave them like you just don't
        nvgTextBox(args.vg, y_position, x_position, wrap_at, to_display.c_str(), NULL);
      }

    }
    else
    {
      drawSliderBackground(vg, slider_background_color);
      drawSlider(vg, 0.6, slider_color);
    }

    nvgRestore(vg);

  }

  void drawSliderBackground(NVGcontext *vg, NVGcolor color)
  {
    drawBar(vg, SLIDER_HEIGHT, color);
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

  NVGcolor getSliderColor(NVGcolor default_color)
  {
    if(module && module->colorful_sliders)
    {
      // If no cable is connected, return the default color
      if(! module->outputs[this->column].isConnected()) return default_color;

      // If a cable is connected, find the top (stacked) cable and return it.
      NVGcolor color = APP->scene->rack->getTopCable(module->output_ports[this->column])->color;
      color.a = 1.0f;  // set alpha
      return(color);
    }
    else // standard sliders
    {
      return(default_color);
    }
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
    if(module)
    {
      // Note: This only works when the slider height is equal to the draw area
      // height.  If it didn't there'd be more thinking to do.  Luckily it is.
      double new_value = (SLIDER_HEIGHT - mouse_position.y) / SLIDER_HEIGHT;
      if (new_value < 0) new_value = 0;
      if (new_value > 1) new_value = 1;

      this->module->sliders[module->selected_bank][column].setValue(new_value);
    }
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

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    if(module) module->is_moused_over_slider = false;
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover& e) override {

    e.consume(this);

    if(module)
    {
      module->is_moused_over_slider = true;
      module->moused_over_slider = this->column;
    }
  }

  void step() override {
    TransparentWidget::step();
  }

};
