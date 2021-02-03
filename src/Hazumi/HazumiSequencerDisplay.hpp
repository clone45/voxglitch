struct HazumiSequencerDisplay : TransparentWidget
{
  Hazumi *module;
  Vec drag_position;
  bool mouse_lock = false;
  int old_row = -1;
  int old_column = -1;

  int alpha_fades[8] = {255,255,255,255,255,255,255,255};

  void HazumiDisplay()
  {
    box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    // Debugging code for draw area, which often has to be set experimentally
    /*
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
    nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
    nvgFill(vg);
    */

    if(module)
    {
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

          if(module->hazumi_sequencer.ball_locations[column] == row)
          {
            // nvgFillColor(vg, nvgRGB(97, 143, 170));  // paint ball

            // If the ball has triggered an output, paint it brighter
            if(module->hazumi_sequencer.stored_trigger_results[column] == true)
            {
              alpha_fades[column] = 160;
              module->hazumi_sequencer.stored_trigger_results[column] = false;
            }
            nvgFillColor(vg, nvgRGBA(97, 143, 170, alpha_fades[column]));

            // This assumes a consistent draw frequency. I should probably
            // replace this with code that accurately fades out the alpha at
            // a exact rate.
            alpha_fades[column] += 6;

            if(alpha_fades[column] > 255) alpha_fades[column] = 255;
          }

          nvgFill(vg);

        }
      }
    }
    //
    // Paint static content for library display
    //
    // !! BE CAREFUL: If you're making changes here, make sure that you're not
    // editing the wrong piece of code        >>>>>> WARNING <<<<<<
    //
    else
    {
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

    }

    nvgRestore(vg);
  }

  std::pair<unsigned int, unsigned int> getRowAndColumnFromVec(Vec position)
  {
    unsigned int row = 17 - (position.y / (CELL_HEIGHT + CELL_PADDING));
    unsigned int column = position.x / (CELL_WIDTH + CELL_PADDING);

    row = clamp(row, 1, SEQUENCER_ROWS);
    column = clamp(column, 0, SEQUENCER_COLUMNS - 1);

    return {row,column};
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

          unsigned int row, column;
          std::tie(row, column) = getRowAndColumnFromVec(e.pos);

          module->hazumi_sequencer.column_heights[column] = row;

          // Store the initial drag position
          drag_position = e.pos;
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
      int row, column;
      std::tie(row, column) = getRowAndColumnFromVec(drag_position);

      if((row != old_row) || (column != old_column))
      {
        // this->setSequencerCell(row, column, this->cell_edit_value);
        module->hazumi_sequencer.column_heights[column] = row;

        old_row = row;
        old_column = column;
      }
    }
    else
    {
      this->mouse_lock = false;
    }
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
