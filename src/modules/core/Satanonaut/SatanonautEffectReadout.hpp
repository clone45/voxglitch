/*
struct SatanonautEffectReadout : TransparentWidget
{
	Satanonaut *module;
	std::shared_ptr<Font> font;

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		std::string text_to_display = "2";

		if(module)
		{
			text_to_display = "#" + std::to_string(module->selected_effect);
		}

    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
  	if (font) {
  		nvgFontSize(args.vg, 40);
  		nvgFontFaceId(args.vg, font->handle);
  		nvgTextLetterSpacing(args.vg, 0);
  		nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 0xff));
  		nvgText(args.vg, 15, 15, text_to_display.c_str(), NULL);
    }

		nvgRestore(args.vg);
	}

};
*/
