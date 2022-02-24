struct ParamEditorDisplay : TransparentWidget
{
  Scalar110 *module;
  Vec drag_position;
  unsigned int selected_parameter = 0;
  double bar_width = (LCD_DISPLAY_WIDTH / NUMBER_OF_STEPS) - BAR_HORIZONTAL_PADDING;

  ParamEditorDisplay()
  {
    // The bounding box needs to be a little deeper than the visual
    // controls to allow mouse drags to indicate '0' (off) column heights,
    // which is why 16 is being added to the draw height to define the
    // bounding box.
    box.size = Vec(LCD_DISPLAY_WIDTH, LCD_DISPLAY_HEIGHT + 16);
  }

  void drawLayer(const DrawArgs& args, int layer) override
  {
  	if (layer == 1)
    {
      const auto vg = args.vg;
      double value;
      NVGcolor bar_color;

      // Save the drawing context to restore later
      nvgSave(vg);

      if(module)
      {
        /*
        double range_low = voltage_ranges[module->selected_voltage_sequencer->voltage_range_index][0];
        double range_high = voltage_ranges[module->selected_voltage_sequencer->voltage_range_index][1];

        if(range_low < 0 && range_high > 0) draw_from_center = true;
        */

        //
        // Display the pattern
        //
        for(unsigned int i=0; i < NUMBER_OF_STEPS; i++)
        {
          value = module->selected_track->getParameter(i, selected_parameter);

          // Draw grey background bar
          // drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, getParameter(unsigned int selected_step, unsigned int parameter_number)); // background

          // Draw bars for the parameter values
          bar_color = nvgRGBA(255, 255, 255, 150);
          if(value > 0) drawBar(vg, i, (value * LCD_DISPLAY_HEIGHT), LCD_DISPLAY_HEIGHT, bar_color);
        }

      }
      else // Draw a demo sequence so that the sequencer looks nice in the library selector
      {
        /*
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
        */
      }

      //drawOverlay(vg, OVERLAY_WIDTH, DRAW_AREA_HEIGHT);

      nvgRestore(vg);
    }

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
  void editBar(Vec mouse_position)
  {
    if(module)
    {

      int clicked_bar_x_index = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
      int clicked_y = LCD_DISPLAY_HEIGHT - mouse_position.y;

      clicked_bar_x_index = clamp(clicked_bar_x_index, 0, NUMBER_OF_STEPS - 1);
      clicked_y = clamp(clicked_y, 0, LCD_DISPLAY_HEIGHT);

      // convert the clicked_y position to a double between 0 and 1
      double value = (double) clicked_y / (double) LCD_DISPLAY_HEIGHT;

      // TODO: Set parameter here
      module->selected_track->setParameter(clicked_bar_x_index, this->selected_parameter, value);
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


};
