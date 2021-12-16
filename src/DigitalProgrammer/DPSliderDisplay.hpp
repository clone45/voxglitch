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
  DPSlider *slider;
  Vec drag_position;

  DPSliderDisplay(DPSlider *slider)
  {
    // The bounding box needs to be a little deeper than the visual
    // controls to allow mouse drags to indicate '0' (off) column heights,
    // which is why 16 is being added to the draw height to define the
    // bounding box.
    this->slider = slider;
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
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
        nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
        nvgFill(vg);
        // -----------------------------------------

        // unsigned int column = 1;

        double value = slider->getValue();

        drawSliderBackground(vg, nvgRGBA(60, 60, 64, 255));   
        drawSlider(vg, value, nvgRGBA(120, 120, 120, 255));
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
    nvgRect(vg, 0.0, DRAW_AREA_HEIGHT - y, SLIDER_WIDTH, y);
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
    /*
    double slider_width = (DRAW_AREA_WIDTH / NUMBER_OF_SLIDERS) - SLIDER_HORIZONTAL_PADDING;
    int clicked_slider_x_index = mouse_position.x / (slider_width + SLIDER_HORIZONTAL_PADDING);
    */

    // Note: This only works when the slider height is equal to the draw area
    // height.  If it didn't there'd be more thinking to do.  Luckily it is.
    double new_value = (SLIDER_HEIGHT - mouse_position.y) / SLIDER_HEIGHT;
    if (new_value < 0) new_value = 0;
    if (new_value > 1) new_value = 1;
    // clicked_bar_x_index = clamp(clicked_bar_x_index, 0, MAX_SEQUENCER_STEPS - 1);
    // new_value = clamp(new_value, 0.0, 1.0);

    // module->selected_voltage_sequencer->setValue(clicked_bar_x_index, clicked_y);

    // Tooltip drawing is done in the draw method
    /*
    draw_tooltip = true;
    draw_tooltip_index = clicked_bar_x_index;
    draw_tooltip_y = clicked_y;
    tooltip_value = module->selected_voltage_sequencer->getOutput(clicked_bar_x_index);
    */
    this->slider->setValue(new_value);
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

/*
  void onHover(const event::Hover &e) override
  {
    if(module->frozen)
    {
      int bar_x_index = e.pos.x / (bar_width + BAR_HORIZONTAL_PADDING);

      // change step here
      module->selected_voltage_sequencer->setPosition(bar_x_index);
      module->selected_gate_sequencer->setPosition(bar_x_index);
    }
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    if(keypressRight(e))
    {
      module->selected_voltage_sequencer->shiftRight();
      if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->shiftRight();
    }

    if(keypressLeft(e))
    {
      module->selected_voltage_sequencer->shiftLeft();
      if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->shiftLeft();
    }

    if(keypressUp(e))
    {
      int bar_x_index = e.pos.x / (bar_width + BAR_HORIZONTAL_PADDING);
      double value = module->selected_voltage_sequencer->getValue(bar_x_index);

      // (.01 * (214 / 10)), where 214 is the bar height and 10 is the max voltage
      value = value + (.01 * (214.0 / 10.0));
      // value = clamp(value, 0.0, DRAW_AREA_HEIGHT);
      if (value > DRAW_AREA_HEIGHT) value = DRAW_AREA_HEIGHT;

      module->selected_voltage_sequencer->setValue(bar_x_index, value);

      module->tooltip_timer = module->sample_rate * 2; // show tooltip for 2 seconds
      tooltip_value = roundf((value / DRAW_AREA_HEIGHT) * 1000) / 100;
      draw_tooltip_index = bar_x_index;
      draw_tooltip_y = value;
    }

    if(keypressDown(e))
    {
      int bar_x_index = e.pos.x / (bar_width + BAR_HORIZONTAL_PADDING);
      double value = module->selected_voltage_sequencer->getValue(bar_x_index);

      // (.01 * (214 / 10)), where 214 is the bar height and 10 is the max voltage
      value = value - (.01 * (214.0 / 10.0));
      // value = clamp(value, 0.0, DRAW_AREA_HEIGHT);
      if (value > DRAW_AREA_HEIGHT) value = DRAW_AREA_HEIGHT;

      module->selected_voltage_sequencer->setValue(bar_x_index, value);

      module->tooltip_timer = module->sample_rate * 2; // show tooltip for 2 seconds
      tooltip_value = roundf((value / DRAW_AREA_HEIGHT) * 1000) / 100;
      draw_tooltip_index = bar_x_index;
      draw_tooltip_y = value;
    }

    // Randomize single sequence by hovering over and pressing 'r'

    if(e.key == GLFW_KEY_R && e.action == GLFW_PRESS)
    {
      // Do not randomize if CTRL-r is pressed.  That's for randomizing everything
      if((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL)
      {
        module->selected_voltage_sequencer->randomize();
        if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->randomize();
      }
    }


    // Send a gate out at the currently selected sequence when pressing "g"
    if(e.key == GLFW_KEY_G && e.action == GLFW_PRESS)
    {
      if((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL) // Ignore control-g
      {
        module->forceGateOut();
      }
    }

    if(e.key == GLFW_KEY_ESCAPE && e.action == GLFW_PRESS)
    {
      module->selected_voltage_sequencer->clear();
      if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->clear();
    }
  }
  */
};
