struct LabelDisplay : TransparentWidget
{
  Scalar110 *module;
  std::shared_ptr<Font> font;
  unsigned int knob_number = 0;
  NVGcolor primary_color = nvgRGB(255, 215, 20);;
  NVGcolor background_color = nvgRGB(0, 0, 0);;

  LabelDisplay(unsigned int knob_number)
  {
    this->knob_number = knob_number;
    box.size = Vec(PARAMETER_LABEL_WIDTH, PARAMETER_LABEL_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    nvgSave(vg);

    std::string text_to_display = "Parameter";

    if(module)
    {
      text_to_display = module->selected_track->engine->getKnobLabel(knob_number);

      if(this->module->selected_parameter == this->knob_number) {
        primary_color = nvgRGB(0, 0, 0);
        background_color = nvgRGB(255, 215, 20);
      }
      else {
        primary_color = nvgRGB(255, 215, 20);
        background_color = nvgRGB(0, 0, 0);
      }
    }
    // Draw label background
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, PARAMETER_LABEL_WIDTH, PARAMETER_LABEL_HEIGHT);
    nvgFillColor(args.vg, background_color);
    nvgFill(vg);

    // Draw label text
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
  	if (font) {
      nvgFontSize(vg, 12);
      nvgFontFaceId(vg, font->handle);
      nvgTextAlign(vg, NVG_ALIGN_CENTER);
      nvgTextLetterSpacing(vg, -1);
      nvgFillColor(vg, primary_color);
      nvgText(vg, 29.0, 13, text_to_display.c_str(), NULL);
  	}

    nvgRestore(vg);
  }

  /*
  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      // Vec mouse_position = e.pos;
      this->module->selected_parameter = this->knob_number;
    }
  }
  */
};
