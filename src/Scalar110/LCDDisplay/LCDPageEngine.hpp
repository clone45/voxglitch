struct LCDPageEngine : LCDPage
{
	std::string text_to_display = "";
  unsigned int window_start = 0;
  unsigned int window_end = 0;

	void draw(NVGcontext *vg) override
	{
    // Set font information
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
    if (font) {
  		nvgFontSize(vg, 10);
  		nvgFontFaceId(vg, font->handle);
  		nvgTextLetterSpacing(vg, 0);
      nvgFillColor(vg, nvgRGBA(0, 0, 0, 0xff));
    }

    // ACTUAL BUG: Sometimes "module" is null, some time it's not!

		if(module)
		{
			text_to_display = "";

      window_start = 0;
      window_end = NUMBER_OF_ENGINES;

      for(unsigned int i = 0; i < 3; i++)
      {

        text_to_display = ENGINE_NAMES[i]; // Defined in defines.h
        text_to_display.resize(22);

        if(i == module->selected_track->getEngine())
        {
          nvgFillColor(vg, nvgRGBA(255, 215, 20, 0xff));
        }
        else
        {
          nvgFillColor(vg, nvgRGBA(136, 116, 19, 0xff));
        }

        // nvgFillColor(vg, nvgRGBA(136, 116, 19, 0xff));
        nvgText(vg, 0, 6.3 + ((i - window_start) * 16), text_to_display.c_str(), NULL);

      }

		}
    else
    {
      // Here's where I display dummy data for library view

    }  // end of else
	} // end of draw method



  void onButton(Vec position) override
  {

  }

  void onDragMove(Vec position) override
  {

  }
};
