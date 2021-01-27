struct StringedDisplay : TransparentWidget
{
  Stringed *module;
  Vec drag_position;
  bool mouse_lock = false;
  Vec old_drag_position;

  StringedDisplay()
  {
    box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    // Debugging code for draw area, which often has to be set experimentally
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
    nvgFillColor(vg, nvgRGB(0, 0, 0));
    nvgFill(vg);

    if(module)
    {
      nvgBeginPath(vg);
      nvgStrokeWidth(vg, 0.5f);
      nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
      nvgMoveTo(vg, STRING_1_X, 0);
      nvgLineTo(vg, STRING_1_X, 400);
      nvgStroke(vg);

      nvgBeginPath(vg);
      nvgStrokeWidth(vg, 0.5f);
      nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
      nvgMoveTo(vg, STRING_2_X, 0);
      nvgLineTo(vg, STRING_2_X, 400);
      nvgStroke(vg);

      nvgBeginPath(vg);
      nvgStrokeWidth(vg, 0.5f);
      nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
      nvgMoveTo(vg, STRING_3_X, 0);
      nvgLineTo(vg, STRING_3_X, 400);
      nvgStroke(vg);

      /*
      for(unsigned int column=0; column < SEQUENCER_COLUMNS; column++)
      {
        for(unsigned int row=0; row < SEQUENCER_ROWS; row++)
        {
          nvgBeginPath(vg);
          nvgRect(vg, (column * CELL_WIDTH) + (column * CELL_PADDING), ((SEQUENCER_ROWS - row - 1) * CELL_HEIGHT) + ((SEQUENCER_ROWS - row - 1) * CELL_PADDING), CELL_WIDTH, CELL_HEIGHT);

          // Default color for inactive square
          nvgFillColor(vg, nvgRGB(220, 220, 220));

          row = clamp(row, 0, SEQUENCER_ROWS - 1);
          column = clamp(column, 0, SEQUENCER_COLUMNS - 1);

          if(module->hazumi_sequencer.column_heights[column] > row) nvgFillColor(vg, nvgRGB(200, 200, 200)); // paint height indicator
          if(module->hazumi_sequencer.ball_locations[column] == row) nvgFillColor(vg, nvgRGB(97, 143, 170));  // paint ball

          nvgFill(vg);

        }
      }
      */
    }
    //
    // Paint static content for library display
    //
    // !! BE CAREFUL: If you're making changes here, make sure that you're not
    // editing the wrong piece of code        >>>>>> WARNING <<<<<<
    //
    else
    {
      /*
      unsigned int lib_ball_locations[8] = {
        5,3,3,1,4,12,0,2
      };

      unsigned int lib_column_heights[8] = {
        8,9,9,4,16,12,0,8
      };

      for(unsigned int column=0; column < SEQUENCER_COLUMNS; column++)
      {
        for(unsigned int row=0; row < SEQUENCER_ROWS; row++)
        {
          nvgBeginPath(vg);
          nvgRect(vg, (column * CELL_WIDTH) + (column * CELL_PADDING), ((SEQUENCER_ROWS - row - 1) * CELL_HEIGHT) + ((SEQUENCER_ROWS - row - 1) * CELL_PADDING), CELL_WIDTH, CELL_HEIGHT);

          // Default color for inactive square
          nvgFillColor(vg, nvgRGB(220, 220, 220));

          row = clamp(row, 0, SEQUENCER_ROWS - 1);
          column = clamp(column, 0, SEQUENCER_COLUMNS - 1);

          if(lib_column_heights[column] > row) nvgFillColor(vg, nvgRGB(200, 200, 200)); // paint height indicator
          if(lib_ball_locations[column] == row) nvgFillColor(vg, nvgRGB(97, 143, 170));  // paint ball

          nvgFill(vg);
        }
      }
      */

    }

    nvgRestore(vg);
  }

  void onButton(const event::Button &e) override
  {
    if(isMouseInDrawArea(e.pos))
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        e.consume(this);

        if(this->mouse_lock == false)
        {
          this->mouse_lock = true;

          // Store the initial drag position
          drag_position = e.pos;
          old_drag_position = drag_position;
        }

      }
      else if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
      {
        this->mouse_lock = false;
      }
    }
  }

  void onDragMove(const event::DragMove &e) override
  {
    TransparentWidget::onDragMove(e);

    double zoom = std::pow(2.f, settings::zoom);
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    if(isMouseInDrawArea(drag_position))
    {
      if(((old_drag_position.x < STRING_1_X) && (drag_position.x > STRING_1_X)) || ((old_drag_position.x > STRING_1_X) && (drag_position.x < STRING_1_X)))
      {
        module->triggered_string[0] = true;
      }

      if(((old_drag_position.x < STRING_2_X) && (drag_position.x > STRING_2_X)) || ((old_drag_position.x > STRING_2_X) && (drag_position.x < STRING_2_X)))
      {
        module->triggered_string[1] = true;
      }

      if(((old_drag_position.x < STRING_3_X) && (drag_position.x > STRING_3_X)) || ((old_drag_position.x > STRING_3_X) && (drag_position.x < STRING_3_X)))
      {
        module->triggered_string[2] = true;
      }

    }
    else
    {
      this->mouse_lock = false;
    }

    old_drag_position = drag_position;
  }

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover& e) override {
    TransparentWidget::onHover(e);
  }

  bool isMouseInDrawArea(Vec position)
  {
    if(position.x < 0) return(false);
    if(position.y < 0) return(false);
    if(position.x >= DRAW_AREA_WIDTH) return(false);
    if(position.y >= DRAW_AREA_HEIGHT) return(false);
    return(true);
  }

  void step() override {
    TransparentWidget::step();
  }

};
