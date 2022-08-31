struct VoltageSequencerDisplayXP : SequencerDisplay
{
  DigitalSequencerXP *module;

  bool draw_tooltip = false;
  double draw_tooltip_index = -1.0;
  double draw_tooltip_y = -1.0;
  double tooltip_value = 0.0;
  bool shift_key = false;
  bool ctrl_key = false;

  int previous_shift_sequence_column = 0;
  int shift_sequence_column = 0;

  int previous_control_sequence_column = 0;
  int control_sequence_column = 0;

  VoltageSequencerDisplayXP()
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
      double value;
      NVGcolor bar_color;
      bool draw_from_center = false;

      // Save the drawing context to restore later
      nvgSave(vg);

      if(module)
      {
        double range_low = module->selected_voltage_sequencer->voltage_ranges[module->selected_voltage_sequencer->voltage_range_index][0];
        double range_high = module->selected_voltage_sequencer->voltage_ranges[module->selected_voltage_sequencer->voltage_range_index][1];

        if(range_low < 0 && range_high > 0) draw_from_center = true;

        //
        // Display the pattern
        //
        for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
        {
          value = module->selected_voltage_sequencer->getValue(i);

          // Draw grey background bar
          if(i < module->selected_voltage_sequencer->getLength()) {
            bar_color = brightness(bright_background_color, settings::rackBrightness);
          }
          else {
            bar_color = brightness(dark_background_color, settings::rackBrightness);
          }

          drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, bar_color); // background

          if(i == module->selected_voltage_sequencer->getPlaybackPosition())
          {
            bar_color = current_step_highlight_color;
          }
          else if(i < module->selected_voltage_sequencer->getLength())
          {
            bar_color = lesser_step_highlight_color;
          }
          else
          {
            bar_color = default_step_highlight_color;
          }

          // Draw bars for the sequence values
          if(value > 0) drawBar(vg, i, (value * DRAW_AREA_HEIGHT), DRAW_AREA_HEIGHT, bar_color);

          // Highlight the sequence playback column
          if(i == module->selected_voltage_sequencer->getPlaybackPosition())
          {
            drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, sequence_position_highlight_color);
          }
        }

        // Draw a horizontal 0-indicator if the range is not symmetrical
        if(draw_from_center)
        {
          // This calculation for y takes advance of the fact that all
          // ranges that would have the 0-guide visible are symmetric, so
          // it will need updating if non-symmetric ranges are added.
          double y = DRAW_AREA_HEIGHT / 2.0;

          nvgBeginPath(vg);
          nvgRect(vg, 1, y, (DRAW_AREA_WIDTH - 2), 1.0);
          nvgFillColor(vg, nvgRGBA(240, 240, 255, 40));
          nvgFill(vg);
        }

        // Draw label, if there is one
        std::string to_display = module->labels[module->selected_sequencer_index];

        if(to_display != "")
        {
          nvgFontSize(args.vg, 14);
          nvgTextLetterSpacing(args.vg, 0);
          nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
          nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
          float x_position = DRAW_AREA_HEIGHT / 2;
          float y_position = 16;
          float wrap_at = 275.0; // Just throw your hands in the air!  And wave them like you just don't 274.0
          nvgTextBox(args.vg, x_position, y_position, wrap_at, to_display.c_str(), NULL);
        }

      }
      else // Draw a demo sequence so that the sequencer looks nice in the library selector
      {
        double demo_sequence[32] = {100.0,100.0,93.0,80.0,67.0,55.0,47.0,44.0,43.0,44.0,50.0,69.0,117.0,137.0,166,170,170,164,148,120,105,77,65,41,28,23,22,22,28,48,69,94};

        for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
        {
          // Draw blue background bars
          drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, nvgRGBA(60, 60, 64, 255));

          // Draw bar for value at i
          drawBar(vg, i, demo_sequence[i], DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 150));

          // Highlight active step
          if(i == 5) drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 20));
        }
      }

      drawVerticalGuildes(vg, DRAW_AREA_HEIGHT);
      drawOverlay(vg, OVERLAY_WIDTH, DRAW_AREA_HEIGHT);

      if(module)
      {
        if(module->tooltip_timer > 0) draw_tooltip = true;

        if(draw_tooltip)
        {
          drawTooltip(vg);
          draw_tooltip = false;
        }
      }

      nvgRestore(vg);

    }

  }

  void drawTooltip(NVGcontext *vg)
  {
    nvgSave(vg);

    double x_offset = 3.0;
    double y = std::max(60.0, draw_tooltip_y);
    double x = ((draw_tooltip_index * bar_width) + (draw_tooltip_index * BAR_HORIZONTAL_PADDING)) + bar_width + x_offset;

    if(draw_tooltip_index > 26) x = x - bar_width - TOOLTIP_WIDTH - (x_offset * 2) - BAR_HORIZONTAL_PADDING;
    y = DRAW_AREA_HEIGHT - y + 30;

    // Draw box for containing text
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, TOOLTIP_WIDTH, TOOLTIP_HEIGHT, 2);
    nvgFillColor(vg, nvgRGBA(20, 20, 20, 250));
    nvgFill(vg);

    // Set up font style
    nvgFontSize(vg, 13);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 0xff));
    nvgTextAlign(vg, NVG_ALIGN_CENTER);
    nvgTextLetterSpacing(vg, -1);

    // Display text
    std::string display_string = std::to_string(tooltip_value);
    display_string = display_string.substr(0,4);
    nvgText(vg, x + 16.5, y + 14, display_string.c_str(), NULL);

    nvgRestore(vg);
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
      double bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
      int clicked_bar_x_index = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
      int clicked_y = DRAW_AREA_HEIGHT - mouse_position.y;

      clicked_bar_x_index = clamp(clicked_bar_x_index, 0, MAX_SEQUENCER_STEPS - 1);
      clicked_y = clamp(clicked_y, 0, DRAW_AREA_HEIGHT);

      // convert the clicked_y position to a double between 0 and 1
      double value = (double) clicked_y / (double) DRAW_AREA_HEIGHT;

      module->selected_voltage_sequencer->setValue(clicked_bar_x_index, value);

      // Tooltip drawing is done in the draw method
      draw_tooltip = true;
      draw_tooltip_index = clicked_bar_x_index;
      draw_tooltip_y = clicked_y;
      tooltip_value = module->selected_voltage_sequencer->getOutput(clicked_bar_x_index);
    }
  }

  void startShiftSequences(Vec mouse_position)
  {
    int clicked_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
    this->previous_shift_sequence_column = clicked_column;
    this->shift_sequence_column = clicked_column;
  }

  void startControlSequence(Vec mouse_position)
  {
    int clicked_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
    clicked_column = clamp(clicked_column, 0, MAX_SEQUENCER_STEPS);
    module->selected_gate_sequencer->setLength(clicked_column);
    module->selected_voltage_sequencer->setLength(clicked_column);
  }

  void dragShiftSequences(Vec mouse_position)
  {
    if(module)
    {
      int drag_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
      int shift_offset = drag_column - this->shift_sequence_column;

      while(shift_offset < 0)
      {
        module->selected_gate_sequencer->shiftLeft();
        module->selected_voltage_sequencer->shiftLeft();
        shift_offset ++;
      }

      while(shift_offset > 0)
      {
        module->selected_gate_sequencer->shiftRight();
        module->selected_voltage_sequencer->shiftRight();
        shift_offset --;
      }

      this->shift_sequence_column = drag_column;
    }
  }

  void dragControlSequence(Vec mouse_position)
  {
    if(module)
    {
      int drag_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
      drag_column = clamp(drag_column, 0, MAX_SEQUENCER_STEPS);
      module->selected_gate_sequencer->setLength(drag_column);
      module->selected_voltage_sequencer->setLength(drag_column);
    }
  }

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      drag_position = e.pos;

      if(this->shift_key == true)
      {
        startShiftSequences(drag_position);
      }
      else if(this->ctrl_key == true)
      {
        startControlSequence(drag_position);
      }
      else
      {
        this->editBar(e.pos);
      }
    }
  }

  void onDragMove(const event::DragMove &e) override
  {
    TransparentWidget::onDragMove(e);
    float zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    if(this->shift_key == true)
    {
      dragShiftSequences(drag_position);
    }
    else if(this->ctrl_key == true)
    {
      dragControlSequence(drag_position);
    }
    else
    {
      editBar(drag_position);
    }
  }

  void onHover(const event::Hover &e) override
  {
    if(module && module->frozen)
    {
      int bar_x_index = e.pos.x / (bar_width + BAR_HORIZONTAL_PADDING);

      // change step here
      module->selected_voltage_sequencer->setPosition(bar_x_index);
      module->selected_gate_sequencer->setPosition(bar_x_index);
    }

    TransparentWidget::onHover(e);
    e.consume(this);
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    if(!module) return;

    this->shift_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT);
    this->ctrl_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL);

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

  void onLeave(const LeaveEvent &e) override
  {
    shift_key = false;
    ctrl_key = false;
  }

};
