struct LengthReadoutDisplay : TransparentWidget
{
  GlitchSequencer *module;
  std::shared_ptr<Font> font;

  LengthReadoutDisplay()
  {
    font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    nvgSave(vg);

    std::string text_to_display = "16";

    if(module)
    {
      text_to_display = std::to_string(module->sequencer.length);
    }

    nvgFontSize(vg, 12);
    nvgFontFaceId(vg, font->handle);
    nvgTextAlign(vg, NVG_ALIGN_CENTER);
    nvgTextLetterSpacing(vg, -1);
    nvgFillColor(vg, nvgRGB(3, 3, 3));
    nvgText(vg, 5, 5, text_to_display.c_str(), NULL);

    nvgRestore(vg);

  }
};
