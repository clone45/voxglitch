struct GrooveboxBlueLight : BlueLight {
  GrooveBox *module;
  bool is_moused_over = false;
  Vec drag_position;
  float diameter = 20.0;
  float radius = diameter / 2.0;

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      drag_position = this->box.pos + Vec(this->box.size[0] / 2, 0);
      BlueLight::onButton(e);
    }
  }

  void onEnter(const event::Enter &e) override
  {
    this->is_moused_over = true;
    BlueLight::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    this->is_moused_over = false;
    BlueLight::onLeave(e);
  }

  void onHover(const event::Hover& e) override {
    e.consume(this);
    this->is_moused_over = true;
  }

  void step() override {
    BlueLight::step();
  }

  void onDragMove(const event::DragMove &e) override
  {
    float space_between_buttons = (button_positions[1][0] - button_positions[0][0]);

    if(module && module->shift_key)
    {
      float zoom = getAbsoluteZoom();
      drag_position = drag_position.plus(e.mouseDelta.div(zoom));

      int amount = -1 * (drag_position.x / space_between_buttons);

      if(amount != 0)
      {
        if(module->control_key) {
          module->shiftAllTracks(amount);
        }
        else {
          module->shiftTrack(amount);
        }

        drag_position.x = e.mouseDelta.div(zoom).x;
      }
    }
  }
};
