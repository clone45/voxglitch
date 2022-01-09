struct CopyPasteLabel : TransparentWidget
{
	DigitalProgrammer *module;
	std::shared_ptr<Font> font;

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		std::string text_to_display = "Copy/Paste";

		if(module)
		{
			text_to_display = "COPY SELECTED";

			if(module->copy_paste_mode)
			{
				text_to_display = "CLICK BANK(S)";
			}
		}

    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
  	if (font) {
  		nvgFontSize(args.vg, 11);
  		nvgFontFaceId(args.vg, font->handle);
  		nvgTextLetterSpacing(args.vg, 0);
  		nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 0xff));
    }

		// void nvgTextBox(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end);
		nvgTextBox(args.vg, 0, 0, 700, text_to_display.c_str(), NULL);

		nvgRestore(args.vg);
	}

};
