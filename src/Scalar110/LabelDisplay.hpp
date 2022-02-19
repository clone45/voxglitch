struct LabelDisplay : TransparentWidget
{
  Scalar110 *module;
  std::shared_ptr<Font> font;
  unsigned int knob_number = 0;

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    nvgSave(vg);

    std::string text_to_display = "Parameter";

    if(module)
    {
      text_to_display = module->selected_track->engine->getKnobLabel(knob_number);
    }

    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
  	if (font) {
      nvgFontSize(vg, 12);
      nvgFontFaceId(vg, font->handle);
      nvgTextAlign(vg, NVG_ALIGN_CENTER);
      nvgTextLetterSpacing(vg, -1);
      nvgFillColor(vg, nvgRGB(3, 3, 3));
      nvgText(vg, 0, 0, text_to_display.c_str(), NULL);
  	}

    nvgRestore(vg);

  }
};
