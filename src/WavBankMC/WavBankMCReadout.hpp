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
  		nvgFontSize(args.vg, 10);
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
        // When there are too many sample filenames to fit on the front panel,
        // then we'll show a window into the sample list.  Here's where the
        // window start and end are computed.

        unsigned int window_start = 0;
        unsigned int window_end = module->samples.size();

        // If there are more samples than can naturally fit in the display, then
        // we'll do some extra work to scroll the panel list if necessary.
        if(! (module->samples.size() < NUMBER_OF_SAMPLE_DISPLAY_ROWS))
        {
          unsigned int half_display_location = (NUMBER_OF_SAMPLE_DISPLAY_ROWS / 2);
          unsigned int number_of_loaded_samples = module->samples.size();

          if(module->selected_sample_slot > half_display_location)
          {
            window_start = module->selected_sample_slot - half_display_location;
            window_end = module->selected_sample_slot + half_display_location + 1;

            if(window_end > number_of_loaded_samples)
            {
              window_start = number_of_loaded_samples - NUMBER_OF_SAMPLE_DISPLAY_ROWS;
              window_end = number_of_loaded_samples;
            }

            if(window_end > number_of_loaded_samples) window_end = number_of_loaded_samples;
          }
          // If the selected sample is near the beginning, then make sure to
          // adjust the window end to the maximum viewable samples
          else
          {
            window_start = 0;
            window_end = NUMBER_OF_SAMPLE_DISPLAY_ROWS;
          }
        }

        for(unsigned int i = window_start; i < window_end; i++)
        {
          text_to_display = module->samples[i].display_name;
          text_to_display.resize(22);

          if(i == module->selected_sample_slot)
          {
            nvgFillColor(args.vg, nvgRGBA(255, 215, 20, 0xff));
          }
          else
          {
            nvgFillColor(args.vg, nvgRGBA(136, 116, 19, 0xff));
          }

          nvgText(args.vg, 0, 4 + ((i - window_start) * 16), text_to_display.c_str(), NULL);
        }
      }
		}
    else
    {
      // Here's where I display dummy samples for viewing in the sample library

      std::string dummy_filenames[20] = {
        "16-bit-version.wav",
        "SO_HU_floor_tom_blown_loud.wav",
        "SO_HU_floor_tom_blown_soft.wav",
        "SO_HU_floor_tom_loud.wav",
        "SO_HU_floor_tom_soft.wav",
        "SO_JP_perc_chappa_single_muted_mf.wav",
        "SO_JP_taiko_cage_large_single_f.wav",
        "SO_JP_taiko_cage_low_flam_mf.wav",
        "SO_JP_taiko_cage_low_smack_ff.wav",
        "SO_JP_taiko_cage_medium_single_mf.wav",
        "SO_JP_taiko_click_top_mf.wav",
        "SO_JP_taiko_solo_nagado_rim_mp.wav",
        "SO_JP_taiko_solo_nagado_single_mf.wav",
        "SO_JP_taiko_solo_ojime_double_f.wav",
        "SO_JP_taiko_solo_ojime_single_mf.wav",
        "SO_JP_taiko_water_drum_single.wav",
        "SO_MSIC_tom_boogieroto.wav",
        "SO_MSIC_tom_clearverb.wav",
        "SO_MSIC_tom_deepfake.wav",
        "SO_MSIC_tom_discobug.wav"
      };

      for(unsigned int i = 0; i < 20; i++)
      {
        text_to_display = dummy_filenames[i];
        text_to_display.resize(22);

        if(i == 12)
        {
          nvgFillColor(args.vg, nvgRGBA(255, 215, 20, 0xff));
        }
        else
        {
          nvgFillColor(args.vg, nvgRGBA(136, 116, 19, 0xff));
        }

        nvgText(args.vg, 0, 4 + (i * 16), text_to_display.c_str(), NULL);
      }
    }
	}
};
