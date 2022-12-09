/*
struct KodamaGridWidget : TransparentWidget
{
  Kodama *module;

  KodamaGridWidget()
  {
    // box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
    box.size = Vec((COLS * (CELL_WIDTH + CELL_PADDING)) - CELL_PADDING, (ROWS * (CELL_HEIGHT + CELL_PADDING)) - CELL_PADDING);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    // Debugging code for draw area, which often has to be set experimentally
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(130, 10, 120, 100));
    nvgFill(vg);

    if (module)
    {
      for (unsigned int column = 0; column < COLS; column++)
      {
        for (unsigned int row = 0; row < ROWS; row++)
        {
          float x = (column * CELL_WIDTH) + (column * CELL_PADDING);
          float y = ((ROWS - row - 1) * CELL_HEIGHT) + ((ROWS - row - 1) * CELL_PADDING);

          // Draw square
          nvgBeginPath(vg);
          nvgRect(vg, x, y, CELL_WIDTH, CELL_HEIGHT);

          // if (module->grid_data[column][row] == true)
          if (module->gate_sequencers[row].getValue(column) == true)
          {
            nvgFillColor(vg, nvgRGB(200, 200, 200));
          }
          else
          {
            if(column == module->gate_sequencers[row].getPlaybackPosition())
            {
                nvgFillColor(vg, nvgRGB(100, 100, 100));
            }
            else
            {
                nvgFillColor(vg, nvgRGB(80, 80, 80));
            }
          }

          nvgFill(vg);
        }
      }
    }

    nvgRestore(vg);
  }

  std::pair<unsigned int, unsigned int> getRowAndColumnFromVec(Vec position)
  {
    unsigned int row = ROWS - (position.y / (CELL_HEIGHT + CELL_PADDING));
    unsigned int column = position.x / (CELL_WIDTH + CELL_PADDING);

    row = clamp(row, 0, ROWS - 1);
    column = clamp(column, 0, COLS - 1);

    return {row, column};
  }

  void onButton(const event::Button &e) override
  {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        e.consume(this);

        unsigned int row, column;
        std::tie(row, column) = getRowAndColumnFromVec(e.pos);

        module->gate_sequencers[row].toggleValue(column);
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

  void onHover(const event::Hover &e) override
  {
    TransparentWidget::onHover(e);
  }

  void step() override
  {
    TransparentWidget::step();
  }
};

*/