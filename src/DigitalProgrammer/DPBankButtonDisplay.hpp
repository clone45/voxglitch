struct DPBankButtonDisplay : TransparentWidget
{
  unsigned int button_index = 0;
  DigitalProgrammer *module;
  bool paste_highlight = false;

  DPBankButtonDisplay(unsigned int bank_button_index) // pass in vector and button id?
  {
    button_index = bank_button_index;
  }

  void draw(const DrawArgs& args) override
  {
    const auto vg = args.vg;
    nvgSave(vg);

    NVGcolor button_default_color = nvgRGBA(31,39,42,255);
    NVGcolor highlight_color = nvgRGBA(45, 61, 60, 255);
    NVGcolor active_mini_map_color = nvgRGBA(174,255,223,255);
    NVGcolor inactive_mini_map_color = nvgRGBA(174,255,223,255);
    NVGcolor paste_mini_map_color = nvgRGBA(97,86,105,255);
    // NVGcolor unhighligted_background = nvgRGBA(31,39,42,255);

    if(module)
    {
      if (module->selected_bank == button_index) {
        // Draw selected bank button
        drawButton(vg, highlight_color);
        drawMiniMap(vg, active_mini_map_color);
      }
      else if (module->is_moused_over_bank && (module->mouse_over_bank == button_index) && ((module->bank_interaction_mode == COPY_MODE) || (module->bank_interaction_mode == CLEAR_MODE))) {
        drawMiniMap(vg, paste_mini_map_color); // draw special mouse-over highlight while in copy/paste mode
      }
      else if (module->is_moused_over_bank && (module->mouse_over_bank == button_index)) { // draw mouse-over highlight
        drawButton(vg, highlight_color);
        drawMiniMap(vg, active_mini_map_color);
      }
      else
      {
        if(paste_highlight) {
          drawMiniMap(vg, paste_mini_map_color);
        }
        else {
          drawButton(vg, button_default_color);
          drawMiniMap(vg, inactive_mini_map_color); // draw standard background
        }
      }

      if(! (module->bank_interaction_mode == COPY_MODE)) paste_highlight = false;
    }
    else // for display in the user library
    {
      drawButton(vg, button_default_color);
    }

    nvgRestore(vg);
  } // end drawLayer

  void drawMiniMap(NVGcontext *vg, NVGcolor color)
  {
    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      float slider_value = module->sliders[button_index][column].getValue();
      float x = (BANK_BUTTON_WIDTH / NUMBER_OF_SLIDERS) * column;
      float y = BANK_BUTTON_HEIGHT;
      float width = (BANK_BUTTON_WIDTH / NUMBER_OF_SLIDERS);
      float height = -1 * (slider_value * BANK_BUTTON_HEIGHT);

      if(height < 0)
      {
        nvgBeginPath(vg);
        nvgRect(vg, x, y, width, height);
        nvgFillColor(vg, color);
        nvgFill(vg);
      }
    }
  }

  void drawButton(NVGcontext *vg, NVGcolor color)
  {
    nvgBeginPath(vg);
    nvgRect(vg, 0.0, 0.0, BANK_BUTTON_WIDTH, BANK_BUTTON_HEIGHT);
    nvgFillColor(vg, color);
    nvgFill(vg);
  } // end drawButton

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);

      if(module->bank_interaction_mode == COPY_MODE)
      {
        module->copyBank(module->copy_bank_id, this->button_index);
        paste_highlight = true;
      }
      else if (module->bank_interaction_mode == CLEAR_MODE)
      {
        module->clearBank(this->button_index);
      }
      else if (module->bank_interaction_mode == RANDOMIZE_MODE)
      {
        module->randomizeBank(this->button_index);
      }
      else // SELECT_MODE
      {
        module->selected_bank = this->button_index;
      }
    }
  } // end onButton

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    module->is_moused_over_bank = false;
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover& e) override {
    e.consume(this);
    module->is_moused_over_bank = true;
    module->mouse_over_bank = this->button_index;
  }

  void step() override {
    TransparentWidget::step();
  }

}; // end struct
