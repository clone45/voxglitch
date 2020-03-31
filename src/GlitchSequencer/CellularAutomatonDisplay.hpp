struct CellularAutomatonDisplay : TransparentWidget
{
  GlitchSequencer *module;
  Vec drag_position;
  bool mouse_lock = false;
  bool cell_edit_value = true;
  int old_row = -1;
  int old_column = -1;

  CellularAutomatonDisplay()
  {
    box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    if(module)
    {
      // testing draw area
      /*
      nvgBeginPath(vg);
      nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
      nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
      nvgFill(vg);
      */

      for(unsigned int row=0; row < SEQUENCER_ROWS; row++)
      {
        for(unsigned int column=0; column < SEQUENCER_COLUMNS; column++)
        {
          nvgBeginPath(vg);
          nvgRect(vg, (column * CELL_WIDTH) + (column * CELL_PADDING), (row * CELL_HEIGHT) + (row * CELL_PADDING), CELL_WIDTH, CELL_HEIGHT);

          // bool cell_is_alive = (module->edit_mode) ? module->sequencer.pattern[row][column] : module->sequencer.state[row][column];

          // Default color for inactive square
          nvgFillColor(vg, nvgRGB(55, 55, 55));

          // When in edit mode, the pattern that's being edited will be bright white
          // and the underlying animation will continue to be shown but at a dim gray
          switch(module->mode)
          {
            case PLAY_MODE:
            if(module->sequencer.seed[row][column]) nvgFillColor(vg, nvgRGB(80, 80, 80));
            if(module->sequencer.state[row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));
            break;

            case EDIT_SEED_MODE:
            if(module->sequencer.state[row][column]) nvgFillColor(vg, nvgRGB(65, 65, 65));
            if(module->sequencer.seed[row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));
            break;

            case EDIT_TRIGGERS_MODE:
            if(module->selected_trigger_group_index >= 0)
            {
              if(module->sequencer.state[row][column]) nvgFillColor(vg, nvgRGB(65, 65, 65));
              bool cell_contains_trigger = module->sequencer.triggers[module->selected_trigger_group_index][row][column];
              bool is_triggered = module->sequencer.state[row][column];
              if(cell_contains_trigger) nvgFillColor(vg, nvgRGB(140, 140, 140));
              if(cell_contains_trigger && is_triggered) nvgFillColor(vg, nvgRGB(255, 255, 255));
            }
            break;
          }

          nvgFill(vg);
        }
      }
    }
    // Paint static content for library display
    else
    {
      CellularAutomatonSequencer ca;

      for(unsigned int row=0; row < SEQUENCER_ROWS; row++)
      {
        for(unsigned int column=0; column < SEQUENCER_COLUMNS; column++)
        {
          nvgBeginPath(vg);
          nvgRect(vg, (column * CELL_WIDTH) + (column * CELL_PADDING), (row * CELL_HEIGHT) + (row * CELL_PADDING), CELL_WIDTH, CELL_HEIGHT);

          nvgFillColor(vg, nvgRGB(55, 55, 55)); // Default color for inactive square
          if(ca.seed[row][column]) nvgFillColor(vg, nvgRGB(255, 255, 255));

          nvgFill(vg);
        }
      }
    }

    nvgRestore(vg);
  }


  std::pair<int, int> getRowAndColumnFromVec(Vec position)
  {
    int row = position.y / (CELL_HEIGHT + CELL_PADDING);
    int column = position.x / (CELL_WIDTH + CELL_PADDING);

    return {row, column};
  }

  void setSequencerCell(unsigned int row, unsigned int column, bool value)
  {
    module->sequencer.seed[row][column] = this->cell_edit_value;

    // If the sequencer is at the first step, also update the current "state"
    // The first "state" of the sequencer should always mirror the pattern
    if(module->sequencer.position == 0) module->sequencer.state[row][column] = this->cell_edit_value;
  }

  void onButton(const event::Button &e) override
  {
    e.consume(this);

    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      if(this->mouse_lock == false)
      {
        this->mouse_lock = true;

        int row, column;
        std::tie(row, column) = getRowAndColumnFromVec(e.pos);

        if(module->mode == EDIT_SEED_MODE)
        {
          // Store the value that's being set for later in case the user
          // drags to set ("paints") additional triggers
          this->cell_edit_value = ! module->sequencer.seed[row][column];

          // Set the cell value in the sequencer
          this->setSequencerCell(row, column, this->cell_edit_value);
        }

        if(module->mode == EDIT_TRIGGERS_MODE && module->selected_trigger_group_index >= 0)
        {
          // Store the value that's being set for later in case the user
          // drags to set ("paints") additional triggers
          this->cell_edit_value = ! module->sequencer.triggers[module->selected_trigger_group_index][row][column];
          module->sequencer.triggers[module->selected_trigger_group_index][row][column] = this->cell_edit_value;
        }

        // Store the initial drag position
        drag_position = e.pos;
      }
    }
    else if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
    {
      this->mouse_lock = false;
    }
  }

  void onDragMove(const event::DragMove &e) override
  {
    TransparentWidget::onDragMove(e);

    double zoom = std::pow(2.f, settings::zoom);
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    int row, column;
    std::tie(row, column)  = getRowAndColumnFromVec(drag_position);

    if((row != old_row) || (column != old_column))
    {
      if(module->mode == EDIT_SEED_MODE)
      {
        this->setSequencerCell(row, column, this->cell_edit_value);
      }

      if(module->mode == EDIT_TRIGGERS_MODE && module->selected_trigger_group_index >= 0)
      {
        module->sequencer.triggers[module->selected_trigger_group_index][row][column] = this->cell_edit_value;
      }

      old_row = row;
      old_column = column;
    }
  }

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
    if(this->module->mode == PLAY_MODE) this->module->mode = EDIT_SEED_MODE;
  }

  void onLeave(const event::Leave &e) override
  {
    TransparentWidget::onLeave(e);
    if(this->module->mode == EDIT_SEED_MODE) this->module->mode = PLAY_MODE;
  }

  void onHover(const event::Hover& e) override {
    TransparentWidget::onHover(e);
    e.consume(this);
  }
};
