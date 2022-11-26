struct RangeGrabberRightWidget : TransparentWidget
{
  GrooveBox *module;
  bool is_moused_over = false;
  // legacy values expected by callers
  float diameter = 20.0;
  float radius = diameter / 2.0;
  float height = 18.0;
  float width = 34.0;
  Vec drag_position;

  RangeGrabberRightWidget()
  {
    box.size = Vec(width, height);
  }


  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Draw grabber
    if(is_moused_over && module) {

      nvgSave(vg);
      nvgBeginPath(vg);

      this->box.pos = Vec(button_positions[module->selected_track->range_end][0] - width/2, this->box.pos.y);

      nvgRoundedRect(vg, box.size.x - width, box.size.y - height, width, height, 2.0);
      nvgFillColor(vg, nvgRGB(83,92,91));

      nvgFill(vg);
      nvgRestore(vg);
    }
  }

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      drag_position = this->box.pos;
    }
  }

  void onEnter(const event::Enter &e) override
  {
		glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));

    this->is_moused_over = true;
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    glfwSetCursor(APP->window->win, NULL);

    this->is_moused_over = false;
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover& e) override {
    e.consume(this);
    this->is_moused_over = true;
  }

  void step() override {
    TransparentWidget::step();
  }

  void onDragMove(const event::DragMove &e) override
  {
    float zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    int quantized_x = ((drag_position.x - button_positions[0][0]) + width) / (button_positions[1][0] - button_positions[0][0]);
    quantized_x = clamp(quantized_x, 0, NUMBER_OF_STEPS - 1);

    if((unsigned int) quantized_x > module->selected_track->range_start) module->selected_track->range_end = quantized_x;
  }
};


struct RangeGrabberLeftWidget : TransparentWidget
{
  GrooveBox *module;
  bool is_moused_over = false;
  // legacy values expected by callers
  float diameter = 20.0;
  float radius = diameter / 2.0;
  float height = 18.0;
  float width = 34.0;
  Vec drag_position;

  RangeGrabberLeftWidget()
  {
    box.size = Vec(width, height);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Draw grabber
    if(is_moused_over && module) {

      nvgSave(vg);
      nvgBeginPath(vg);

      this->box.pos = Vec(button_positions[module->selected_track->range_start][0] - width/2, this->box.pos.y);

      nvgRoundedRect(vg, box.size.x - width, box.size.y - height, width, height, 2.0);
      nvgFillColor(vg, nvgRGB(83,92,91));

      nvgFill(vg);
      nvgRestore(vg);
    }
  }


  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      drag_position = this->box.pos;
    }
  }

  void onEnter(const event::Enter &e) override
  {
		glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));

    this->is_moused_over = true;
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    glfwSetCursor(APP->window->win, NULL);

    this->is_moused_over = false;
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover& e) override {
    e.consume(this);
    this->is_moused_over = true;
  }

  void step() override {
    TransparentWidget::step();
  }

  void onDragMove(const event::DragMove &e) override
  {
    float zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    int quantized_x = ((drag_position.x - button_positions[0][0]) + width) / (button_positions[1][0] - button_positions[0][0]);
    quantized_x = clamp(quantized_x, 0, NUMBER_OF_STEPS - 1);

    if((unsigned int) quantized_x < module->selected_track->range_end) module->selected_track->range_start = quantized_x;
  }
};
