// The Glitch Sequencer's screen: the cellular-automaton grid, plus the two
// control columns that make editing EXPLICIT (ported from the vxsynth web
// version of this module):
//
//   • LEFT: a tab column — [SEED] then trigger groups 1..8, each in its own
//     hue.  Click a tab to edit that layer (the selection lives in
//     module->edit_layer); click the active tab again to just watch.  A tab
//     flashes when its group's gate fires.  This replaces the old implicit
//     hover-to-edit-seed behavior and the 8 panel bezel buttons.
//   • RIGHT: 16 memory buttons (2x8, column-major like the GrooveBox).  Click
//     to switch memories; right-click for Copy / Paste / Clear.  The buttons
//     lock (and light amber) while a cable is patched into the MEM input.
//   • BOTTOM: a position bar with one tick per generation.
//
// While a layer is being edited the live automaton keeps drawing underneath
// as ghost gray, so playback and editing are never confused for each other.

struct CellularAutomatonDisplay : VoxglitchWidget
{
  GlitchSequencer *module = nullptr;

  Vec drag_position;
  bool mouse_lock = false;
  bool cell_edit_value = true;
  int old_row = -1;
  int old_column = -1;

  // ── Geometry (carves up box.size, which comes from the panel's "screen" rect)

  float cellSize()
  {
    return (box.size.x - (2 * SCREEN_PADDING + TAB_COLUMN_WIDTH + 2 * SCREEN_GAP + MEMORY_COLUMN_WIDTH)) / SEQUENCER_COLUMNS;
  }
  float gridX() { return SCREEN_PADDING + TAB_COLUMN_WIDTH + SCREEN_GAP; }
  float gridY() { return SCREEN_PADDING; }
  float gridWidth() { return cellSize() * SEQUENCER_COLUMNS; }
  float gridHeight() { return cellSize() * SEQUENCER_ROWS; }
  float tabHeight() { return gridHeight() / (NUMBER_OF_TRIGGER_GROUPS + 1); }
  float tabWidth() { return TAB_COLUMN_WIDTH - 2.0; }

  // The tab and memory columns are centered within the space each occupies
  // (between the screen edge and the grid).
  float tabX() { return (gridX() - tabWidth()) / 2.0; }
  float memX() { return gridX() + gridWidth() + ((box.size.x - (gridX() + gridWidth())) - MEMORY_COLUMN_WIDTH) / 2.0; }

  // ── Drawing ────────────────────────────────────────────────────────────────

  void drawText(NVGcontext *vg, float x, float y, const char *text, float size, NVGcolor color)
  {
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
    if(!font) return;

    nvgFontSize(vg, size);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 0);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, color);
    nvgText(vg, x, y, text, NULL);
  }

  void fillRect(NVGcontext *vg, float x, float y, float w, float h, NVGcolor color)
  {
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFillColor(vg, color);
    nvgFill(vg);
  }

  void drawScreen(const DrawArgs &args, GlitchSequencer *m)
  {
    const auto vg = args.vg;

    // When m == nullptr (library browser) draw a watch-mode preview of the
    // default demo pattern.
    CellularAutomatonSequencer preview;

    int edit_layer = m ? m->edit_layer : EDIT_LAYER_WATCH;
    bool (*seed)[SEQUENCER_COLUMNS] = m ? m->sequencer.seed : preview.seed;
    bool (*state)[SEQUENCER_COLUMNS] = m ? m->sequencer.state : preview.state;
    unsigned int position = m ? m->sequencer.position : 0;
    unsigned int length = m ? m->sequencer.length : 16;
    unsigned int active_slot = m ? m->memory_slot_index : 0;
    bool mem_locked = m ? m->memCableIsConnected() : false;
    bool editing = edit_layer >= 0;

    NVGcolor seed_color = gs_rgb(0x6fd3ff);
    NVGcolor live_color = gs_rgb(0x39ff14);
    NVGcolor ghost_color = gs_rgb(0x4a554c);
    NVGcolor amber_color = gs_rgb(0xff9f40);

    float cs = cellSize();
    float gx = gridX();
    float gy = gridY();

    // Screen background
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 4);
    nvgFillColor(vg, gs_rgb(0x161a1f));
    nvgFill(vg);

    //
    // Cells: base, then live state (ghosted while editing), then the layer
    // being edited in its own color, or faint seed dots in watch mode.
    //

    for(unsigned int row = 0; row < SEQUENCER_ROWS; row++)
    {
      for(unsigned int column = 0; column < SEQUENCER_COLUMNS; column++)
      {
        float x = gx + column * cs;
        float y = gy + row * cs;

        fillRect(vg, x + 0.5, y + 0.5, cs - 1.0, cs - 1.0, gs_rgb(0x1d2226));

        bool alive = state[row][column];
        if(alive) fillRect(vg, x + 1.0, y + 1.0, cs - 2.0, cs - 2.0, editing ? ghost_color : live_color);

        if(edit_layer == EDIT_LAYER_SEED)
        {
          if(seed[row][column]) fillRect(vg, x + 1.0, y + 1.0, cs - 2.0, cs - 2.0, seed_color);
        }
        else if(edit_layer >= 1 && m)
        {
          if(m->sequencer.triggers[edit_layer - 1][row][column])
          {
            // Trigger cells draw in the group's own hue: full brightness where
            // the cell is currently alive, dimmed where it's dormant.
            NVGcolor group_color = gs_rgb(GS_GROUP_COLORS[edit_layer - 1]);
            fillRect(vg, x + 1.0, y + 1.0, cs - 2.0, cs - 2.0, alive ? group_color : brightness(group_color, 0.45));
          }
        }
        else if(seed[row][column] && !alive)
        {
          // Watch mode: mark seed cells with a small dot so "seed" and
          // "alive" are visually distinct.
          nvgBeginPath(vg);
          nvgCircle(vg, x + cs / 2.0, y + cs / 2.0, std::max(1.5f, (float)(cs * 0.13)));
          nvgFillColor(vg, nvgTransRGBA(seed_color, 140));
          nvgFill(vg);
        }
      }
    }

    // Grid lines
    nvgBeginPath(vg);
    for(unsigned int column = 0; column <= SEQUENCER_COLUMNS; column++)
    {
      nvgMoveTo(vg, gx + column * cs, gy);
      nvgLineTo(vg, gx + column * cs, gy + gridHeight());
    }
    for(unsigned int row = 0; row <= SEQUENCER_ROWS; row++)
    {
      nvgMoveTo(vg, gx, gy + row * cs);
      nvgLineTo(vg, gx + gridWidth(), gy + row * cs);
    }
    nvgStrokeWidth(vg, 0.5);
    nvgStrokeColor(vg, nvgTransRGBA(gs_rgb(0x2b3138), 128));
    nvgStroke(vg);

    //
    // Position bar: loop progress with one tick per generation
    //

    float bar_y = gy + gridHeight() + 2.0;
    nvgBeginPath(vg);
    nvgRoundedRect(vg, gx, bar_y, gridWidth(), POSITION_BAR_HEIGHT, 2);
    nvgFillColor(vg, gs_rgb(0x12161a));
    nvgFill(vg);

    float fraction = (length > 0) ? ((float) position / (float) length) : 0.0;
    if(fraction > 0)
    {
      nvgBeginPath(vg);
      nvgRoundedRect(vg, gx, bar_y, std::max(2.0f, (float)(gridWidth() * fraction)), POSITION_BAR_HEIGHT, 2);
      nvgFillColor(vg, nvgTransRGBA(live_color, 204));
      nvgFill(vg);
    }

    nvgBeginPath(vg);
    for(unsigned int i = 1; i < length && i < MAX_SEQUENCE_LENGTH; i++)
    {
      float tick_x = gx + gridWidth() * ((float) i / (float) length);
      nvgMoveTo(vg, tick_x, bar_y);
      nvgLineTo(vg, tick_x, bar_y + POSITION_BAR_HEIGHT);
    }
    nvgStrokeWidth(vg, 0.4);
    nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 110));
    nvgStroke(vg);

    //
    // Tab column: [SEED] + trigger groups 1..8
    //

    float th = tabHeight();
    for(unsigned int i = 0; i <= NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      float y = gy + i * th;
      NVGcolor tab_color = (i == 0) ? seed_color : gs_rgb(GS_GROUP_COLORS[i - 1]);
      bool selected = (edit_layer == (int) i);

      nvgBeginPath(vg);
      nvgRoundedRect(vg, tabX(), y + 1.5, tabWidth(), th - 3.0, 3);
      nvgFillColor(vg, selected ? tab_color : gs_rgb(0x232a30));
      nvgFill(vg);

      // Flash when this group's gate fires
      if(i >= 1 && m && m->tab_flash[i - 1] > 0.01)
      {
        nvgBeginPath(vg);
        nvgRoundedRect(vg, tabX(), y + 1.5, tabWidth(), th - 3.0, 3);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, (unsigned char)(178.0 * m->tab_flash[i - 1])));
        nvgFill(vg);
      }

      nvgBeginPath(vg);
      nvgRoundedRect(vg, tabX(), y + 1.5, tabWidth(), th - 3.0, 3);
      nvgStrokeWidth(vg, 1.0);
      nvgStrokeColor(vg, nvgTransRGBA(tab_color, selected ? 255 : 128));
      nvgStroke(vg);

      std::string label = (i == 0) ? "SEED" : std::to_string(i);
      drawText(vg, tabX() + tabWidth() / 2.0, y + th / 2.0, label.c_str(), (i == 0) ? 10.0 : 12.0, selected ? gs_rgb(0x10151a) : tab_color);
    }

    //
    // Memory column: 16 slot buttons, 2 columns x 8 rows, column-major
    // (1..8 down the left, 9..16 down the right, like the GrooveBox)
    //

    float mx = memX();
    NVGcolor on_color = mem_locked ? amber_color : live_color;

    drawText(vg, mx + MEMORY_COLUMN_WIDTH / 2.0, gy + 4.0, "MEM", 8.0, mem_locked ? amber_color : gs_rgb(0x8a96a2));

    float buttons_y = gy + MEMORY_LABEL_HEIGHT;
    float buttons_h = gridHeight() - MEMORY_LABEL_HEIGHT;
    float bw = (MEMORY_COLUMN_WIDTH - MEMORY_BUTTON_GAP) / 2.0;
    float bh = (buttons_h - 7.0 * MEMORY_BUTTON_GAP) / 8.0;

    for(unsigned int i = 0; i < NUMBER_OF_MEMORY_SLOTS; i++)
    {
      unsigned int button_column = i / 8;
      unsigned int button_row = i % 8;
      float bx = mx + button_column * (bw + MEMORY_BUTTON_GAP);
      float by = buttons_y + button_row * (bh + MEMORY_BUTTON_GAP);
      bool on = (i == active_slot);

      nvgBeginPath(vg);
      nvgRoundedRect(vg, bx, by, bw, bh, 2.5);
      nvgFillColor(vg, on ? on_color : gs_rgb(0x232a30));
      nvgFill(vg);

      nvgBeginPath(vg);
      nvgRoundedRect(vg, bx, by, bw, bh, 2.5);
      nvgStrokeWidth(vg, 1.0);
      nvgStrokeColor(vg, on ? on_color : gs_rgb(0x3a444f));
      nvgStroke(vg);

      drawText(vg, bx + bw / 2.0, by + bh / 2.0, std::to_string(i + 1).c_str(), 9.0, on ? gs_rgb(0x10151a) : gs_rgb(0x8a96a2));
    }
  }

  void draw(const DrawArgs &args) override
  {
    // Library browser preview
    if(!module)
    {
      nvgSave(args.vg);
      drawScreen(args, nullptr);
      nvgRestore(args.vg);
    }
  }

  void drawLayer(const DrawArgs &args, int layer) override
  {
    if(layer == 1 && module)
    {
      nvgSave(args.vg);
      drawScreen(args, module);
      nvgRestore(args.vg);
    }

    Widget::drawLayer(args, layer);
  }

  // ── Hit testing ────────────────────────────────────────────────────────────

  // Returns the tab index (0 = SEED, 1..8 = trigger group) or -1
  int tabAt(Vec position)
  {
    if(position.x < tabX() || position.x >= tabX() + tabWidth()) return -1;
    float yy = position.y - gridY();
    if(yy < 0 || yy >= gridHeight()) return -1;
    int tab = (int) (yy / tabHeight());
    if(tab > NUMBER_OF_TRIGGER_GROUPS) tab = NUMBER_OF_TRIGGER_GROUPS;
    return tab;
  }

  // Returns the memory slot index (0..15) or -1
  int memAt(Vec position)
  {
    float mx = memX();
    float buttons_y = gridY() + MEMORY_LABEL_HEIGHT;
    float buttons_h = gridHeight() - MEMORY_LABEL_HEIGHT;
    float bw = (MEMORY_COLUMN_WIDTH - MEMORY_BUTTON_GAP) / 2.0;
    float bh = (buttons_h - 7.0 * MEMORY_BUTTON_GAP) / 8.0;

    for(unsigned int i = 0; i < NUMBER_OF_MEMORY_SLOTS; i++)
    {
      float bx = mx + (i / 8) * (bw + MEMORY_BUTTON_GAP);
      float by = buttons_y + (i % 8) * (bh + MEMORY_BUTTON_GAP);
      if(position.x >= bx && position.x <= bx + bw && position.y >= by && position.y <= by + bh) return i;
    }
    return -1;
  }

  bool cellAt(Vec position, int &row, int &column)
  {
    float cs = cellSize();
    float x = position.x - gridX();
    float y = position.y - gridY();
    if(x < 0 || y < 0 || x >= gridWidth() || y >= gridHeight()) return false;

    row = clamp((int) (y / cs), 0, SEQUENCER_ROWS - 1);
    column = clamp((int) (x / cs), 0, SEQUENCER_COLUMNS - 1);
    return true;
  }

  bool isMouseInGridArea(Vec position)
  {
    if(position.x < gridX()) return(false);
    if(position.y < gridY()) return(false);
    if(position.x >= gridX() + gridWidth()) return(false);
    if(position.y >= gridY() + gridHeight()) return(false);
    return(true);
  }

  // ── Interaction ────────────────────────────────────────────────────────────

  void paintCell(int row, int column)
  {
    if(module->edit_layer == EDIT_LAYER_SEED)
    {
      module->sequencer.seed[row][column] = this->cell_edit_value;

      // If the sequencer is at the first step, also update the current "state".
      // The first "state" of the sequencer should always mirror the seed.
      if(module->sequencer.position == 0) module->sequencer.state[row][column] = this->cell_edit_value;
    }
    else if(module->edit_layer >= 1)
    {
      module->sequencer.triggers[module->edit_layer - 1][row][column] = this->cell_edit_value;
    }
  }

  void showMemoryMenu(unsigned int slot_index)
  {
    GlitchSequencer *module = this->module;

    ui::Menu *menu = createMenu();
    menu->addChild(createMenuLabel("Memory " + std::to_string(slot_index + 1)));
    menu->addChild(createMenuItem("Copy", "", [=]() { module->copyMemory(slot_index); }));
    menu->addChild(createMenuItem("Paste", "", [=]() { module->pasteMemory(slot_index); }, !module->memory_clipboard_filled));
    menu->addChild(createMenuItem("Clear", "", [=]() { module->clearMemory(slot_index); }));
  }

  void onButton(const event::Button &e) override
  {
    if(!module) return;

    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);

      int tab = tabAt(e.pos);
      if(tab >= 0)
      {
        module->toggleEditLayer(tab);
        return;
      }

      int slot = memAt(e.pos);
      if(slot >= 0)
      {
        // The MEM CV owns the selection while its cable is connected
        if(!module->memCableIsConnected()) module->switchMemory(slot);
        return;
      }

      if(module->edit_layer == EDIT_LAYER_WATCH) return;

      if(this->mouse_lock == false)
      {
        int row, column;
        if(cellAt(e.pos, row, column))
        {
          this->mouse_lock = true;

          // Store the value that's being set for later in case the user
          // drags to set ("paints") additional cells
          if(module->edit_layer == EDIT_LAYER_SEED)
          {
            this->cell_edit_value = ! module->sequencer.seed[row][column];
          }
          else
          {
            this->cell_edit_value = ! module->sequencer.triggers[module->edit_layer - 1][row][column];
          }

          paintCell(row, column);

          // Store the initial drag position
          drag_position = e.pos;
          old_row = row;
          old_column = column;
        }
      }
    }
    else if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
    {
      e.consume(this);
      this->mouse_lock = false;
    }
    else if(e.button == GLFW_MOUSE_BUTTON_RIGHT && e.action == GLFW_PRESS)
    {
      // Right-click on a memory button: Copy / Paste / Clear.  Anywhere else,
      // leave the event alone so the module's context menu still opens.
      int slot = memAt(e.pos);
      if(slot >= 0)
      {
        e.consume(this);
        showMemoryMenu(slot);
      }
    }
  }

  void onDragMove(const event::DragMove &e) override
  {
    TransparentWidget::onDragMove(e);

    if(!module || !mouse_lock) return;

    float zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    if(isMouseInGridArea(drag_position))
    {
      int row, column;
      if(cellAt(drag_position, row, column) && ((row != old_row) || (column != old_column)))
      {
        paintCell(row, column);
        old_row = row;
        old_column = column;
      }
    }
    else
    {
      this->mouse_lock = false;
    }
  }

  void onHover(const event::Hover& e) override {
    TransparentWidget::onHover(e);
    e.consume(this);
  }
};
