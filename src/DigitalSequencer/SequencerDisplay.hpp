struct SequencerDisplay : TransparentWidget
{
  DigitalSequencer *module;
  Vec drag_position;
  double bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;

  void onDragStart(const event::DragStart &e) override
  {
    TransparentWidget::onDragStart(e);
  }

  void onDragEnd(const event::DragEnd &e) override
  {
    TransparentWidget::onDragEnd(e);
  }

  void step() override {
    TransparentWidget::step();
  }

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    TransparentWidget::onLeave(e);
  }

  bool keypress(const event::HoverKey &e, int keycode)
  {
    if (e.key == keycode)
    {
      e.consume(this);
      if(e.action == GLFW_PRESS) return(true);
    }
    return(false);
  }

  bool keypressRight(const event::HoverKey &e)
  {
    return(keypress(e, GLFW_KEY_RIGHT));
  }

  bool keypressLeft(const event::HoverKey &e)
  {
    return(keypress(e, GLFW_KEY_LEFT));
  }

  bool keypressUp(const event::HoverKey &e)
  {
    return(keypress(e, GLFW_KEY_UP));
  }

  bool keypressDown(const event::HoverKey &e)
  {
    return(keypress(e, GLFW_KEY_DOWN));
  }

  void drawVerticalGuildes(NVGcontext *vg, double height)
  {
    for(unsigned int i=1; i < 8; i++)
    {
      nvgBeginPath(vg);
      int x = (i * 4 * bar_width) + (i * 4 * BAR_HORIZONTAL_PADDING);
      nvgRect(vg, x, 0, 1, height);
      nvgFillColor(vg, nvgRGBA(240, 240, 255, 40));
      nvgFill(vg);
    }
  }

  void drawBlueOverlay(NVGcontext *vg, double width, double height)
  {
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, width, height);
    nvgFillColor(vg, nvgRGBA(0, 100, 255, 28));
    nvgFill(vg);
  }

  void drawBar(NVGcontext *vg, double position, double height, double container_height, NVGcolor color)
  {
    nvgBeginPath(vg);
    nvgRect(vg, (position * bar_width) + (position * BAR_HORIZONTAL_PADDING), container_height - height, bar_width, height);
    nvgFillColor(vg, color);
    nvgFill(vg);
  }
};
