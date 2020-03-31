struct GoblinsSampleReadout : TransparentWidget
{
	Goblins *module;
	std::shared_ptr<Font> font;

	GoblinsSampleReadout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		std::string text_to_display = "Right click to load sample";

		if(module)
		{
			text_to_display = "#" + std::to_string(module->selected_sample_slot + 1) + ":" + module->samples[module->selected_sample_slot].filename;
			text_to_display.resize(41); // Truncate long text to fit in the display
		}

		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
		nvgText(args.vg, 5, 5, text_to_display.c_str(), NULL);

		nvgRestore(args.vg);
	}

};
