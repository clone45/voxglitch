struct GateSequencerDisplay : SequencerDisplay
{
  bool mouse_lock = false;
  double bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
  int old_drag_bar_x = -1;
  bool trigger_edit_value = false;

  GateSequencerDisplay()
  {
    box.size = Vec(GATES_DRAW_AREA_WIDTH, GATES_DRAW_AREA_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;
    int value;
    double value_height;
    NVGcolor bar_color;

    nvgSave(vg);

    if(module)
    {
      for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
      {
        value = module->selected_gate_sequencer->getValue(i);

        // Draw grey background bar
        if(i < module->selected_gate_sequencer->getLength()) {
          bar_color = nvgRGBA(60, 60, 64, 255);
        }
        else {
          bar_color = nvgRGBA(45, 45, 45, 255);
        }
        drawBar(vg, i, GATE_BAR_HEIGHT, GATES_DRAW_AREA_HEIGHT, bar_color);

        unsigned int playback_position = module->selected_gate_sequencer->getPlaybackPosition();

        if(i == playback_position)
        {
          bar_color = nvgRGBA(255, 255, 255, 250);
        }
        else if(i < module->selected_gate_sequencer->getLength())
        {
          bar_color = nvgRGBA(255, 255, 255, 150);
        }
        else // dim bars past playback position
        {
          bar_color = nvgRGBA(255, 255, 255, 15);
        }

        value_height = (GATE_BAR_HEIGHT * value);
        if(value_height > 0) drawBar(vg, i, value_height, GATES_DRAW_AREA_HEIGHT, bar_color);

        // highlight active column
        if(i == playback_position)
        {
          drawBar(vg, i, GATE_BAR_HEIGHT, GATES_DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 20));
        }
      }
    }
    else // draw demo data for the module explorer
    {
      int demo_sequence[32] = {1,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0};

      for(unsigned int i=0; i < MAX_SEQUENCER_STEPS; i++)
      {
        value = demo_sequence[i];

        // Draw background grey bar
        drawBar(vg, i, GATE_BAR_HEIGHT, GATES_DRAW_AREA_HEIGHT, nvgRGBA(60, 60, 64, 255));

        // Draw value bar
        if (value > 0) drawBar(vg, i, (GATE_BAR_HEIGHT * value), GATES_DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 150));

        // highlight active column
        if(i == 5) drawBar(vg, i, GATE_BAR_HEIGHT, GATES_DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 20));
      }
    }

    drawVerticalGuildes(vg, GATES_DRAW_AREA_HEIGHT);
    drawBlueOverlay(vg, GATES_DRAW_AREA_WIDTH, GATES_DRAW_AREA_HEIGHT);

    nvgRestore(vg);

  }

  void onButton(const event::Button &e) override
  {
    e.consume(this);

    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      if(this->mouse_lock == false)
      {
        this->mouse_lock = true;

        int index = getIndexFromX(e.pos.x);

        // Store the value that's being set for later in case the user
        // drags to set ("paints") additional triggers
        this->trigger_edit_value = ! module->selected_gate_sequencer->getValue(index);

        // Set the trigger value in the sequencer
        module->selected_gate_sequencer->setValue(index, this->trigger_edit_value);

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

    int drag_bar_x_index = getIndexFromX(drag_position.x);

    if(drag_bar_x_index != old_drag_bar_x)
    {
      // setTrigger(drag_bar_x_index, trigger_edit_value);
      module->selected_gate_sequencer->setValue(drag_bar_x_index, trigger_edit_value);
      old_drag_bar_x = drag_bar_x_index;
    }
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    if(keypressRight(e))
    {
      module->selected_gate_sequencer->shiftRight();
      if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_voltage_sequencer->shiftRight();
    }

    if(keypressLeft(e))
    {
      module->selected_gate_sequencer->shiftLeft();
      if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_voltage_sequencer->shiftLeft();
    }

    // Randomize sequence by hovering over and pressing 'r'
    if(keypress(e, GLFW_KEY_R))
    {
      module->selected_gate_sequencer->randomize();
      if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) module->selected_voltage_sequencer->randomize();
    }
  }

  int getIndexFromX(double x)
  {
    return(x / (barWidth() + BAR_HORIZONTAL_PADDING));
  }

  double barWidth()
  {
    return((DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING);
  }

};
