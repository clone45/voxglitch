struct DPBankButtonDisplay : TransparentWidget
{
  unsigned int button_index = 0;
  DigitalProgrammer *module;

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
      if (layer == 1)
      {
        if (module->selected_bank == button_index)
        {
          drawButton(vg, nvgRGBA(156, 167, 185, 255)); // draw forground
        }
        else
        {
          drawButton(vg, nvgRGBA(53, 64, 85, 255)); // draw background
        }
      }
    }

    nvgRestore(vg);
  } // end drawLayer

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
      module->selected_bank = this->button_index;
    }
  } // end onButton
}; // end struct
