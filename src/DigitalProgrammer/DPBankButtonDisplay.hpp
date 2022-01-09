struct DPBankButtonDisplay : TransparentWidget
{
  unsigned int button_index = 0;
  DigitalProgrammer *module;
  bool paste_highlight = false;

  DPBankButtonDisplay(unsigned int bank_button_index) // pass in vector and button id?
  {
    button_index = bank_button_index;
  }

  void drawLayer(const DrawArgs& args, int layer) override
  {
    const auto vg = args.vg;
    nvgSave(vg);

    if(module)
    {
      if(!module->copy_paste_mode) paste_highlight = false;

      if (layer == 1)
      {
        if (module->selected_bank == button_index) {
          drawButton(vg, nvgRGBA(156, 167, 185, 255)); // draw selected bright forground
        }
        else if (module->is_moused_over_bank && (module->mouse_over_bank == button_index) && module->copy_paste_mode) {
          drawButton(vg, nvgRGBA(97, 86, 105, 255)); // draw special mouse-over highlight while in copy/paste mode
        }
        else if (module->is_moused_over_bank && (module->mouse_over_bank == button_index)) {
          drawButton(vg, nvgRGBA(66, 77, 97, 255)); // draw mouse-over highlight
          drawMiniMap(vg, nvgRGBA(156, 167, 185, 255));
        }
        else
        {
          if(paste_highlight) {
            drawButton(vg, nvgRGBA(97, 86, 105, 255)); // draw paste highlight for pasted banks
          }
          else {
            drawButton(vg, nvgRGBA(53, 64, 85, 255)); // draw background
          }
        }
      }
    }

    nvgRestore(vg);
  } // end drawLayer

  void drawMiniMap(NVGcontext *vg, NVGcolor color)
  {
    for(int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      float slider_value = module->sliders[button_index][column].getValue();
      float x = (BANK_BUTTON_WIDTH / NUMBER_OF_SLIDERS) * column;
      float y = BANK_BUTTON_HEIGHT;
      float width = (BANK_BUTTON_WIDTH / NUMBER_OF_SLIDERS);
      float height = -1 * (slider_value * BANK_BUTTON_HEIGHT);

      nvgBeginPath(vg);
      nvgRect(vg, x, y, width, height);
      nvgFillColor(vg, color);
      nvgFill(vg);
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

      if(module->copy_paste_mode)
      {
        module->copyBank(module->copy_bank_id, this->button_index);
        paste_highlight = true;
      }
      else
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
