struct DisplaySampleSelect : Display
{
	std::shared_ptr<Font> font;
	std::string text_to_display = "";
  unsigned int window_start = 0;
  unsigned int window_end = 0;
  SampleBank& sample_bank = SampleBank::get_instance();

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

		if(module)
		{
			text_to_display = "";

      unsigned int number_of_samples = sample_bank.size();

      if(number_of_samples > 0)
      {
        // When there are too many sample filenames to fit on the front panel,
        // then we'll show a window into the sample list.  Here's where the
        // window start and end are computed.

        window_start = 0;
        window_end = number_of_samples;

        unsigned int selected_sample_slot = (module->selected_track->getParameter(module->selected_step, 0) * sample_bank.size());
        selected_sample_slot = clamp(selected_sample_slot, 0, sample_bank.size() - 1);

        // If there are more samples than can naturally fit in the display, then
        // we'll do some extra work to scroll the panel list if necessary.
        if(! (number_of_samples < NUMBER_OF_SAMPLE_DISPLAY_ROWS))
        {
          unsigned int half_display_location = (NUMBER_OF_SAMPLE_DISPLAY_ROWS / 2);

          if(selected_sample_slot > half_display_location)
          {
            window_start = selected_sample_slot - half_display_location;
            window_end = selected_sample_slot + half_display_location + 1;

            //DEBUG(std::to_string(selected_sample_slot).c_str());
            //DEBUG(std::to_string(half_display_location).c_str());

            if(window_end > number_of_samples)
            {
              window_start = number_of_samples - NUMBER_OF_SAMPLE_DISPLAY_ROWS;
              window_end = number_of_samples;
            }
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
          text_to_display = sample_bank.getDisplayName(i);
          text_to_display.resize(22);

          // nvgFillColor(vg, nvgRGBA(136, 116, 19, 0xff));

          if(i == selected_sample_slot)
          {
            nvgFillColor(vg, nvgRGBA(255, 215, 20, 0xff));
          }
          else
          {
            nvgFillColor(vg, nvgRGBA(136, 116, 19, 0xff));
          }

          nvgText(vg, 0, 6.3 + ((i - window_start) * 16), text_to_display.c_str(), NULL);
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

      for(unsigned int i = 0; i < 20; i++)
      {
        text_to_display = dummy_filenames[i];
        text_to_display.resize(22);

        if(i == 12)
        {
          nvgFillColor(vg, nvgRGBA(255, 215, 20, 0xff));
        }
        else
        {
          nvgFillColor(vg, nvgRGBA(136, 116, 19, 0xff));
        }

        nvgText(vg, 0, 4 + (i * 16), text_to_display.c_str(), NULL);
      } // end for loop
    }  // end of else
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


  void onButton(Vec position) override
  {

  }

  void onDragMove(Vec position) override
  {

  }
};
