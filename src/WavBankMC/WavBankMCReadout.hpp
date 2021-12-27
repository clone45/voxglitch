struct WavBankMCReadout : TransparentWidget
{
	WavBankMC *module;
	float text_rotation_angle = -M_PI / 2.0f;
	std::shared_ptr<Font> font;
	std::string text_to_display = "";

	void draw(const DrawArgs &args) override
	{

    // Set font information
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
    if (font) {
  		nvgFontSize(args.vg, 12);
  		nvgFontFaceId(args.vg, font->handle);
  		nvgTextLetterSpacing(args.vg, 0);
  		// nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
      nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 0xff));
  		// nvgRotate(args.vg, text_rotation_angle);
    }

		if(module)
		{
			text_to_display = "";

      /*
			if(module->samples.size() > module->selected_sample_slot)
			{
				text_to_display = module->samples[module->selected_sample_slot].filename;
				text_to_display.resize(30); // truncate long text
			}
      */
      if(module->samples.size() > 0)
      {
        for(unsigned int i = 0; i < module->samples.size(); i++)
        {
          text_to_display = module->samples[i].filename;
          text_to_display.resize(30);

          if(i == module->selected_sample_slot)
          {
            nvgFillColor(args.vg, nvgRGBA(40, 110, 255, 0xff));
          }
          else
          {
            nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 0xff));
          }

          nvgText(args.vg, 0, 5 + (i * 16), text_to_display.c_str(), NULL);
        }
      }
		}
    else
    {
      // Display this in the library
      text_to_display = "Right-Click to Load Samples";
  		nvgText(args.vg, 0, 5, text_to_display.c_str(), NULL);
    }
	}
};
