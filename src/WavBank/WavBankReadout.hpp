struct WavBankReadout : TransparentWidget
{
	WavBank *module;
	std::shared_ptr<Font> font;
	std::string text_to_display = "";
  bool mouse_lock = false;
  unsigned int window_start = 0;
  unsigned int window_end = 0;
  unsigned int hover_row = 0;
  bool show_hover_effect = false;

	void draw(const DrawArgs &args) override
	{

    // Set font information
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
    if (font) {
  		nvgFontSize(args.vg, 11);
  		nvgFontFaceId(args.vg, font->handle);
  		nvgTextLetterSpacing(args.vg, 0);
    }

		if(module)
		{
			text_to_display = "";

      unsigned int number_of_sample = module->sample_players.size();

      if(number_of_sample > 0)
      {
        // When there are too many sample filenames to fit on the front panel,
        // then we'll show a window into the sample list.  Here's where the
        // window start and end are computed.

        window_start = 0;
        window_end = number_of_sample;

        // If there are more samples than can naturally fit in the display, then
        // we'll do some extra work to scroll the panel list if necessary.
        if(! (number_of_sample < NUMBER_OF_SAMPLE_DISPLAY_ROWS))
        {
          unsigned int half_display_location = (NUMBER_OF_SAMPLE_DISPLAY_ROWS / 2);
          unsigned int number_of_loaded_samples = number_of_sample;

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
          text_to_display = module->sample_players[i].sample.display_name;
          text_to_display.resize(22);

          if(i == module->selected_sample_slot || (show_hover_effect && hover_row == i))
          {
            // nvgFillColor(args.vg, nvgRGBA(122, 179, 193, 0xff));
            nvgFillColor(args.vg, nvgRGBA(150, 219, 234, 0xff));
          }
          else
          {
            // nvgFillColor(args.vg, nvgRGBA(73, 107, 116, 0xff));
            nvgFillColor(args.vg, nvgRGBA(102, 151, 163, 0xff));
          }

          nvgText(args.vg, 0, 6.3 + ((i - window_start) * 14), text_to_display.c_str(), NULL);
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
        "Thank_you_Zak_Forrest.wav"
      };

      for(unsigned int i = 0; i < NUMBER_OF_SAMPLE_DISPLAY_ROWS; i++)
      {
        text_to_display = dummy_filenames[i];
        text_to_display.resize(22);

        if(i == 4)
        {
          nvgFillColor(args.vg, nvgRGBA(150, 219, 234, 0xff));
        }
        else
        {
          nvgFillColor(args.vg, nvgRGBA(102, 151, 163, 0xff));
        }

        nvgText(args.vg, 0, 6.3 + ((i - window_start) * 14), text_to_display.c_str(), NULL);
      }
    }
	} // end of draw method
};