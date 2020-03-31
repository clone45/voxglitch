struct Readout : TransparentWidget
{
	Repeater *module;
	std::shared_ptr<Font> font;

	Readout()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		std::string text_to_display = "load sample";

		if(module)
		{
			if(module->any_sample_has_been_loaded == true)
			{
				text_to_display = "#" + std::to_string(module->selected_sample_slot + 1) + ":" + module->samples[module->selected_sample_slot].filename;
				text_to_display.resize(30); // Truncate long text to fit in the display
			}
			else
			{
				text_to_display = "Right-click to load samples";
			}

			if(module->trig_input_is_connected == false)
			{
				text_to_display = "Connect a clock signal to CLK";
			}
		}

		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);
		nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));

		// void nvgTextBox(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end);
		nvgTextBox(args.vg, 5, 5, 700, text_to_display.c_str(), NULL);

		nvgRestore(args.vg);
	}

};
