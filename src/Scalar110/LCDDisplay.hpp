struct LCDDisplay : TransparentWidget
{
  Scalar110 *module;
  Vec drag_position;
  bool mouse_lock = false;

  LCDDisplay()
  {
    box.size = Vec(LCD_DISPLAY_WIDTH, LCD_DISPLAY_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;


    // Save the drawing context to restore later
    nvgSave(vg);

    if(module)
    {
      // Debugging code for draw area, which often has to be set experimentally
      nvgBeginPath(vg);
      nvgRect(vg, 0, 0, LCD_DISPLAY_WIDTH, LCD_DISPLAY_HEIGHT);
      nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
      nvgFill(vg);

      module->selected_track->engine->LCDDraw(args);
    }
    else
    {
      // !! BE CAREFUL: If you're making changes here, make sure that you're not
      // editing the wrong piece of code
      // >>>>>> WARNING <<<<<<
      //
      // Draw empty display for user library
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
        }

        // Not sure of this, but I think that I'm in the right direction
        // ---------------------------------------------------------------
        // call module->selected_track->engine::onButton(e)
        // double zoom = getAbsoluteZoom();
        // click_position = e.mouseDelta.div(zoom);
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

    double zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    if(isMouseInDrawArea(drag_position))
    {
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
    if(position.x >= LCD_DISPLAY_WIDTH) return(false);
    if(position.y >= LCD_DISPLAY_HEIGHT) return(false);
    return(true);
  }

  void step() override {
    TransparentWidget::step();
  }

};
