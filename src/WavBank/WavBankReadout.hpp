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
  		nvgFontSize(args.vg, 9);
  		nvgFontFaceId(args.vg, font->handle);
  		nvgTextLetterSpacing(args.vg, 0);
    }

		if(module)
		{
			text_to_display = "";

      // For debugging
      /*
      nvgBeginPath(args.vg);
      nvgRect(args.vg, 0, 0, READOUT_WIDTH, READOUT_HEIGHT);
      nvgFillColor(args.vg, nvgRGBA(120, 20, 20, 100));
      nvgFill(args.vg);
      */

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
            nvgFillColor(args.vg, nvgRGBA(122, 179, 193, 0xff));
          }
          else
          {
            nvgFillColor(args.vg, nvgRGBA(73, 107, 116, 0xff));
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
          nvgFillColor(args.vg, nvgRGBA(122, 179, 193, 0xff));
        }
        else
        {
          nvgFillColor(args.vg, nvgRGBA(73, 107, 116, 0xff));
        }

        nvgText(args.vg, 0, 6.3 + ((i - window_start) * 14), text_to_display.c_str(), NULL);
      }
    }
	} // end of draw method

  /*
  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);

      if(this->mouse_lock == false)
      {
        unsigned int row = (e.pos.y / 360.0) * NUMBER_OF_SAMPLE_DISPLAY_ROWS;
        if(module->wav_input_not_connected())
        {
          if((row + window_start) < module->number_of_samples)
          {
            module->change_selected_sample(row + window_start);
            module->set_wav_knob_position();
          }
        }
      }
    }
    else if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
    {
      this->mouse_lock = false;
    }
  }
  */
  /*
  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    show_hover_effect = false;
    TransparentWidget::onLeave(e);
  }


  void onHover(const event::Hover& e) override {

    if(module->wav_input_not_connected())
    {
      unsigned int row = (e.pos.y / 360.0) * NUMBER_OF_SAMPLE_DISPLAY_ROWS;

      if((row + window_start) < module->number_of_samples)
      {
        show_hover_effect = true;
        hover_row = row + window_start;
      }
    }

    e.consume(this);
    TransparentWidget::onHover(e);
  }

  void step() override {
    TransparentWidget::step();
  }
    */
};



/*
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
			text_to_display = "";

			if(module->sample_players.size() > module->selected_sample_slot)
			{
				text_to_display = module->sample_players[module->selected_sample_slot].getFilename();
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
*/
