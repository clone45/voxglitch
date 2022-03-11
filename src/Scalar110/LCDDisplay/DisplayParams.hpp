struct DisplayParams : Display
{
  Vec drag_position;
  double bar_width = (LCD_DISPLAY_WIDTH / NUMBER_OF_STEPS) - BAR_HORIZONTAL_PADDING;

  void draw(NVGcontext *vg) override
  {
      double value;
      NVGcolor bar_color;

      // Save the drawing context to restore later
      nvgSave(vg);

      if(module)
      {
        // Display the pattern
        for(unsigned int i=0; i < NUMBER_OF_STEPS; i++)
        {
          value = module->selected_track->getParameter(i, module->selected_parameter);

          // Draw grey background bar
          // drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, getParameter(unsigned int selected_step, unsigned int parameter_number)); // background

          // Draw bars for the parameter values
          bar_color = nvgRGB(156, 167, 185);
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

      unsigned int step = clamp(clicked_bar_x_index, 0, NUMBER_OF_STEPS - 1);
      clicked_y = clamp(clicked_y, 0, LCD_DISPLAY_HEIGHT);

      // convert the clicked_y position to a double between 0 and 1
      float value = (double) clicked_y / (double) LCD_DISPLAY_HEIGHT;

      // Set the parameter
      module->selected_track->setParameter(step, module->selected_parameter, value);

      // If the bar being edited is selected, adjust the knob of the parameter
      if(step == module->selected_step) module->setParameterKnobPosition(module->selected_parameter, value);

    }
  }

  void onButton(Vec position) override
  {
    this->editBar(position);
  }

  void onDragMove(Vec position) override
  {
    this->editBar(position);
  }


};
