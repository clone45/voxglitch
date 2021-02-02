struct DualSlopeCanvas : TransparentWidget
{
  DualSlope *module;
  Vec drag_position;
  bool mouse_lock = false;
  int old_row = -1;
  int old_column = -1;

  DualSlopeCanvas()
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
    nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
    nvgFill(vg);

    if(module)
    {
    }
    else
    {
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
          /*
          unsigned int row, column;
          std::tie(row, column) = getRowAndColumnFromVec(e.pos);

          module->hazumi_sequencer.column_heights[column] = row;
          */
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
      /*
      int row, column;
      std::tie(row, column) = getRowAndColumnFromVec(drag_position);

      if((row != old_row) || (column != old_column))
      {
        // this->setSequencerCell(row, column, this->cell_edit_value);
        module->hazumi_sequencer.column_heights[column] = row;

        old_row = row;
        old_column = column;
      }
      */
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
