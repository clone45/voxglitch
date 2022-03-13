struct WavBankReadout : TransparentWidget
{
	WavBank *module;
	float text_rotation_angle = -M_PI / 2.0f;
	std::shared_ptr<Font> font;
	std::string text_to_display = "Right-Click to Load Samples";

	void draw(const DrawArgs &args) override
	{
    text_to_display = "right-click to load samples";

		if(module)
		{
			if(module->samples.size() > module->selected_sample_slot)
			{
				text_to_display = module->samples[module->selected_sample_slot].filename;
				text_to_display.resize(30); // truncate long text
			}
		}

		// Set font information
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Minecraftia-Regular.ttf"));
    if (font) {
  		nvgFontSize(args.vg, 12);
  		nvgFontFaceId(args.vg, font->handle);
  		nvgTextLetterSpacing(args.vg, 0);
  		nvgFillColor(args.vg, nvgRGBA(244, 237, 231, 0xff));
  		nvgRotate(args.vg, text_rotation_angle);
    }

		// Print out the text
		nvgText(args.vg, 0, 9, text_to_display.c_str(), NULL);
	}
};
