struct VoltageSequencerDisplay : SequencerDisplay
{
  bool draw_tooltip = false;
  double draw_tooltip_index = -1.0;
  double draw_tooltip_y = -1.0;
  double tooltip_value = 0.0;

  VoltageSequencerDisplay()
  {
    // The bounding box needs to be a little deeper than the visual
    // controls to allow mouse drags to indicate '0' (off) column heights,
    // which is why 16 is being added to the draw height to define the
    // bounding box.
    box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT + 16);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;
    int value;
    NVGcolor bar_color;
    bool draw_from_center = false;

    // Save the drawing context to restore later
    nvgSave(vg);

    if(module)
    {
      double range_low = voltage_ranges[module->selected_voltage_sequencer->voltage_range_index][0];
      double range_high = voltage_ranges[module->selected_voltage_sequencer->voltage_range_index][1];

      if(range_low < 0 && range_high > 0) draw_from_center = true;

      //
      // Display the pattern
      //
      for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
      {
        value = module->selected_voltage_sequencer->getValue(i);

        // Draw grey background bar
        if(i < module->selected_voltage_sequencer->getLength()) {
          bar_color = nvgRGBA(60, 60, 64, 255);
        }
        else {
          bar_color = nvgRGBA(45, 45, 45, 255);
        }

        drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, bar_color);

        if(i == module->selected_voltage_sequencer->getPlaybackPosition())
        {
          bar_color = nvgRGBA(255, 255, 255, 250);
        }
        else if(i < module->selected_voltage_sequencer->getLength())
        {
          bar_color = nvgRGBA(255, 255, 255, 150);
        }
        else
        {
          bar_color = nvgRGBA(255, 255, 255, 10);
        }

        // Draw bars for the sequence values
        if(value > 0) drawBar(vg, i, value, DRAW_AREA_HEIGHT, bar_color);

        // Highlight the sequence playback column
        if(i == module->selected_voltage_sequencer->getPlaybackPosition())
        {
          drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 20));
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
    drawBlueOverlay(vg, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);

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
    double zoom = std::pow(2.f, settings::zoom);
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));
    editBar(drag_position);
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

    // Randomize sequence by hovering over and pressing 'r'
    if(keypress(e, GLFW_KEY_R))
    {
      module->selected_voltage_sequencer->randomize();
      if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_gate_sequencer->randomize();
    }
  }
};
