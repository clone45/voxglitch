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
        drawButton(vg, nvgRGBA(53, 64, 85, 255)); // draw background
        // if module->selected_bank == button_id then highlight
        // {
        //    drawButton(vg, value, nvgRGBA(156, 167, 185, 255)); // draw forground
        // }
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
      // drag_position = e.pos;
      // this->editBar(e.pos);

      // set the active bank in the module
      // no other steps should need to be taken
    }
  } // end onButton
}; // end struct
