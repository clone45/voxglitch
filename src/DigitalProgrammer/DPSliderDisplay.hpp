struct DPSliderDisplay
{
  DPSliderDisplay()
  {
    // The bounding box needs to be a little deeper than the visual
    // controls to allow mouse drags to indicate '0' (off) column heights,
    // which is why 16 is being added to the draw height to define the
    // bounding box.
    box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT + 16);
  }

  void drawLayer(const DrawArgs& args, int layer) override
  {
  	if (layer == 1)
    {
      const auto vg = args.vg;
      int value;
      NVGcolor bar_color;

      // Save the drawing context to restore later
      nvgSave(vg);

      if(module)
      {
          bar_color = nvgRGBA(60, 60, 64, 255);
          drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, bar_color);
        }
      }
      else // Draw a demo sequence so that the sequencer looks nice in the library selector
      {
        // Draw placeholder graphics for Library
      }
      drawBlueOverlay(vg, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);

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

  void drawBar(NVGcontext *vg, double position, double height, double container_height, NVGcolor color)
  {
    nvgBeginPath(vg);
    nvgRect(vg, (position * bar_width) + (position * BAR_HORIZONTAL_PADDING), container_height - height, bar_width, height);
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
  /*
  void editBar(Vec mouse_position)
  {
    double bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
    int clicked_bar_x_index = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
    int clicked_y = DRAW_AREA_HEIGHT - mouse_position.y;

    clicked_bar_x_index = clamp(clicked_bar_x_index, 0, MAX_SEQUENCER_STEPS - 1);
    clicked_y = clamp(clicked_y, 0, DRAW_AREA_HEIGHT);

    module->selected_voltage_sequencer->setValue(clicked_bar_x_index, clicked_y);

    // Tooltip drawing is done in the draw method
    draw_tooltip = true;
    draw_tooltip_index = clicked_bar_x_index;
    draw_tooltip_y = clicked_y;
    tooltip_value = module->selected_voltage_sequencer->getOutput(clicked_bar_x_index);
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
