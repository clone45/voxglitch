//==============================================================================
// TrackLabelDisplay
//==============================================================================
// Bright orange track name displays that are positioned to
// the right of the track selection buttons
//
struct TrackLabelDisplay : TransparentWidget
{

  GrooveBox *module;
  unsigned int track_number = 0;

  TrackLabelDisplay(unsigned int track_number, float width, float height)
  {
    this->track_number = track_number;
    box.size = Vec(width, height);
  }

  void onDoubleClick(const event::DoubleClick &e) override
  {
#ifdef USING_CARDINAL_NOT_RACK
    GrooveBox *module = this->module;
    unsigned int track_number = this->track_number;
    async_dialog_filebrowser(false, NULL, module->samples_root_dir.c_str(), "Load filename", [module, track_number](char *filename)
                             {
      if(filename)
      {
        fileSelected(module, track_number, std::string(filename));
        free(filename);
      } });
#else
    fileSelected(module, this->track_number, module->selectFileVCV());
#endif
  }

  void onButton(const event::Button &e) override
  {
    if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      module->selectTrack(this->track_number);
    }

    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0)
    {
      createContextMenu();
      e.consume(this);
    }
  }

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    TransparentWidget::onLeave(e);
  }

  static void fileSelected(GrooveBox *module, unsigned int track_number, std::string filename)
  {
    if (filename != "")
    {
      module->sample_players[track_number].loadSample(filename);
      module->loaded_filenames[track_number] = module->sample_players[track_number].getFilename();
      module->setRoot(filename);
    }
  }

  void draw_track_label(std::string label, NVGcontext *vg)
  {
    float text_left_margin = 6;

    // Set up font parameters
    nvgFontSize(vg, 10);
    nvgTextLetterSpacing(vg, 0);
    nvgFillColor(vg, LCDColorScheme::getTextColor());
    
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    float wrap_at = 130.0; // Just throw your hands in the air!  And wave them like you just don't 130.0

    // Compute the number of lines that would be drawn
    const char *end = NULL;
    NVGtextRow rows[3];
    unsigned int max_rows = 3;
    unsigned int number_of_lines = nvgTextBreakLines(vg, label.c_str(), NULL, wrap_at, rows, max_rows);

    if (number_of_lines > 1)
      end = rows[1].end;

    float bounds[4];
    nvgTextBoxBounds(vg, text_left_margin, 10, wrap_at, label.c_str(), end, bounds);

    // Plot the name of the file loaded in the track
    float textHeight = bounds[3];
    nvgTextBox(vg, text_left_margin, (box.size.y / 2.0f) - (textHeight / 2.0f) + 8, wrap_at, label.c_str(), end);
  }

  void draw_focus_overlay(NVGcontext *vg)
  {
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(240, 240, 240, 20));
    nvgFill(vg);
  }

  void drawLayer(const DrawArgs &args, int layer) override
  {

    if (layer == 1)
    {
      const auto vg = args.vg;

      // Save the drawing context to restore later
      nvgSave(vg);

      //
      // Draw track names
      //
      if (module)
      {
        if (!module->lcd_screen_mode == module->TRACK)
        {
          nvgRestore(vg);
          return;
        }

        // Draw track slot background
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        if (module->track_index == this->track_number)
        {
          nvgFillColor(vg, LCDColorScheme::getLightColor());
        }
        else
        {
          nvgFillColor(vg, LCDColorScheme::getDarkColor());
        }
        nvgFill(vg);

        std::string to_display = module->sample_players[track_number].getFilename();

        // If the track name is not empty, then display it
        if ((to_display != "") && (to_display != "[ empty ]"))
        {
          draw_track_label(to_display, vg);
        }
      }
      //
      // Draw placeholder track names for library view
      //
      else
      {
        // Draw track slot background
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, LCDColorScheme::getLightColor());
        nvgFill(vg);

        draw_track_label(PLACEHOLDER_TRACK_NAMES[track_number], vg);
      }
      nvgRestore(vg);
    }
    Widget::drawLayer(args, layer);

  }

  struct ClearTrackStepsMenuItem : MenuItem
  {
    GrooveBox *module;
    unsigned int track_number = 0;

    void onAction(const event::Action &e) override
    {
      module->clearTrackSteps(this->track_number);
    }
  };

  struct ClearTrackParametersMenuItem : MenuItem
  {
    GrooveBox *module;
    unsigned int track_number = 0;

    void onAction(const event::Action &e) override
    {
      module->clearTrackParameters(this->track_number);
    }
  };

  struct ClearTrackMenuItem : MenuItem
  {
    GrooveBox *module;
    unsigned int track_number = 0;

    void onAction(const event::Action &e) override
    {
      module->clearTrack(this->track_number);
    }
  };

  // This code is s copy of the same thing in GrooveBoxWidget.hpp.  I should
  // find some way of removing this code duplication.

  struct LoadSampleMenuItem : MenuItem
  {
    GrooveBox *module;
    unsigned int track_number = 0;

    void onAction(const event::Action &e) override
    {
#ifdef USING_CARDINAL_NOT_RACK
      GrooveBox *module = this->module;
      unsigned int track_number = this->track_number;
      async_dialog_filebrowser(false, NULL, module->samples_root_dir.c_str(), "Load sample", [module, track_number](char *filename)
                               {
      if(filename)
      {
        fileSelected(module, track_number, std::string(filename));
        free(filename);
      } });
#else
      fileSelected(module, this->track_number, module->selectFileVCV());
#endif
    }

    static void fileSelected(GrooveBox *module, unsigned int track_number, std::string filename)
    {
      if (filename != "")
      {
        module->sample_players[track_number].loadSample(filename);
        module->loaded_filenames[track_number] = module->sample_players[track_number].getFilename();
        module->setRoot(filename);
      }
    }
  };

  void createContextMenu()
  {
    GrooveBox *module = dynamic_cast<GrooveBox *>(this->module);
    assert(module);

    if(module)
    {
      ui::Menu *menu = createMenu();

      menu->addChild(createMenuLabel("Track Menu"));

      LoadSampleMenuItem *load_sample_menu_item = createMenuItem<LoadSampleMenuItem>("Load Sample");
      load_sample_menu_item->module = module;
      load_sample_menu_item->track_number = track_number;
      menu->addChild(load_sample_menu_item);

      menu->addChild(new MenuSeparator());

      ClearTrackStepsMenuItem *clear_track_steps_menu_item = createMenuItem<ClearTrackStepsMenuItem>("Clear Track Steps");
      clear_track_steps_menu_item->module = module;
      clear_track_steps_menu_item->track_number = track_number;
      menu->addChild(clear_track_steps_menu_item);

      ClearTrackParametersMenuItem *clear_track_parameters_menu_item = createMenuItem<ClearTrackParametersMenuItem>("Reset Track Parameters");
      clear_track_parameters_menu_item->module = module;
      clear_track_parameters_menu_item->track_number = track_number;
      menu->addChild(clear_track_parameters_menu_item);

      ClearTrackMenuItem *clear_track_menu_item = createMenuItem<ClearTrackMenuItem>("Clear and Reset Both");
      clear_track_menu_item->module = module;
      clear_track_menu_item->track_number = track_number;
      menu->addChild(clear_track_menu_item);
    }
  }

};

//
//==============================================================================
// TrackSampleNudge
//==============================================================================
//
// TODO: Nudge buttons sometimes don't do anything.  This happens, for example,
// if you're at the first sample in a folder and try to nudge "up" (previous).
// It would be nice if there were some indication that the nudge button is 
// essentially disabled (maybe a darker color or something).
//

struct TrackSampleNudge : TransparentWidget
{
  GrooveBox *module;
  unsigned int track_number = 0;

  int direction = 1;

  TrackSampleNudge(unsigned int track_number, float width, float height)
  {
    this->track_number = track_number;
    box.size = Vec(width, height);
  }

  void onButton(const event::Button &e) override
  {
    if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      module->selectTrack(this->track_number);

      std::string path = module->sample_players[track_number].getPath();
      std::string directory = rack::system::getDirectory(path);
      std::string filename = module->sample_players[track_number].getFilename();

      std::vector<std::string> directory_list = system::getEntries(directory);
      std::vector<std::string> wav_files;

      // Folders might contain things that aren't .wav files, and we need to
      // weed those out. In order to do that, we iterate over the directory list
      // and populate a new vector called "wav_files".
      for (auto entry : directory_list)
      {
        if (
            (rack::string::lowercase(system::getExtension(entry)) == "wav") ||
            (rack::string::lowercase(system::getExtension(entry)) == ".wav"))
        {
          wav_files.push_back(entry);
        }
      }

      // Now that we have a clean list, search for the currently selected
      // wav file.  If we find it (which we should), find the next sample.
      for (unsigned i = 0; i < wav_files.size(); i++)
      {
        std::string filename_in_directory = rack::system::getFilename(wav_files[i]);

        if (filename_in_directory.compare(filename) == 0) // Found it!
        {
          int index = i + direction;
          index = clamp(index, 0, wav_files.size() - 1);
          fileSelected(this->module, this->track_number, wav_files[index]);
          break;
        }
      }

      e.consume(this);
    }
  }

  void onEnter(const event::Enter &e) override
  {
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover &e) override
  {
    TransparentWidget::onHover(e);
    e.consume(this);
  }

  static void fileSelected(GrooveBox *module, unsigned int track_number, std::string filename)
  {
    if (filename != "")
    {
      module->sample_players[track_number].loadSample(filename);
      module->loaded_filenames[track_number] = module->sample_players[track_number].getFilename();
      module->setRoot(filename);
    }
  }

  void drawLayer(const DrawArgs &args, int layer) override
  {
    if (layer == 1)
    {
      const auto vg = args.vg;

      // Save the drawing context to restore later
      nvgSave(vg);

      //
      // Draw nudge button
      //
      if (module)
      {
        if (!module->lcd_screen_mode == module->TRACK)
        {
          nvgRestore(vg);
          return;
        }

        // Draw nudge rectangle background
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        if (module->track_index == this->track_number)
        {
          nvgFillColor(vg, LCDColorScheme::getLightColor());
        }
        else
        {
          nvgFillColor(vg, LCDColorScheme::getDarkColor());
        }
        nvgFill(vg);

        drawArrow(vg, direction);
      }
      else
      {
        // Draw nudge rectangle background and arrow for the module browser
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, LCDColorScheme::getLightColor());
        nvgFill(vg);

        drawArrow(vg, direction);
      }
      nvgRestore(vg);
    }
    Widget::drawLayer(args, layer);
  }

  void drawArrow(NVGcontext *vg, int direction)
  {
    nvgFontSize(vg, 10);

    nvgFillColor(vg, LCDColorScheme::getStrongHighlightOverlay());
    if (module && module->track_index == this->track_number) nvgFillColor(vg, LCDColorScheme::getTextColor());

    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

    // Plot character
    if (direction < 0)
    {
      nvgText(vg, 10.0, 6.0, "▲", NULL);
    }
    else
    {
      nvgText(vg, 10.0, 7.0, "▼", NULL);
    }
  }
};

//
//==============================================================================
// LCDTrackDisplay
//==============================================================================
//
struct LCDTrackDisplay : LCDDisplay
{
  LCDTrackDisplay(GrooveBox *module)
  {
    this->module = module;

    box.pos.x = this->box_pos_x;
    box.pos.y = this->box_pos_y;
    box.size = Vec(this->box_width, this->box_height);

    //
    // MAKE ADJUSTMENTS HERE
    //
    // A while back, I had hard coded almost all of the position values for the
    // track display.  Then, the design and widget placement was completely 
    // changed, and I would have had to go through and recalculate all of the
    // positions.  Instead, I decided to spend a bit more effort in creating a 
    // positioning system that would be more flexible.
    // 
    // I decided that certain things should be "fixed", and some should be "fluid":
    // -> Paddings and the nudge button sizes should be fixed
    // -> Track labels should be "fluid"
    //
    // If you wish to change the box sizes, please update the padding and
    // allow the box sizes to adjust based on the available space.
    //
    // Here are the variables that I suggest modifying if the LCDTrackDisplay 
    // ever changes dimensions:
    float nudge_button_padding = 1.0;
    float nudge_button_width = 20.0;
    float horizontal_padding_between_columns = display_padding;
    float vertical_padding_between_rows = 3.0;

    // Calculate track label width and height
    float track_label_width = (this->box_width - (2.0 * display_padding) - horizontal_padding_between_columns - (2.0 * nudge_button_padding) - (2.0 * nudge_button_width)) / 2.0;
    float track_label_height = (this->box_height - (2.0 * display_padding) - (3.0 * vertical_padding_between_rows)) / 4.0;

    // Calculate nudge button height
    float nudge_button_height = (track_label_height / 2.0) - (nudge_button_padding / 2.0);

    float x = 0;
    float y = 0;

    for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      if (i >= 4)
      {
        x = display_padding + track_label_width + nudge_button_padding + nudge_button_width + horizontal_padding_between_columns;
        y = display_padding + ((i - 4) * (track_label_height + vertical_padding_between_rows));
      }
      else
      {
        x = display_padding;
        y = display_padding + (i * (track_label_height + vertical_padding_between_rows));
      }

      //
      // Add track label boxes
      //
      TrackLabelDisplay *track_label_display = new TrackLabelDisplay(i, track_label_width, track_label_height);
      track_label_display->setPosition(Vec(x, y));
      track_label_display->module = module;
      addChild(track_label_display);

      //
      // Add nudge-up, nudge-down buttons
      //
      TrackSampleNudge *track_sample_nudge_up = new TrackSampleNudge(i, nudge_button_width, nudge_button_height);
      track_sample_nudge_up->setPosition(Vec(x + track_label_width + nudge_button_padding, y)); // 162
      track_sample_nudge_up->module = module;
      track_sample_nudge_up->direction = -1;
      addChild(track_sample_nudge_up);

      TrackSampleNudge *track_sample_nudge_down = new TrackSampleNudge(i, nudge_button_width, nudge_button_height);
      track_sample_nudge_down->setPosition(Vec(x + track_label_width + nudge_button_padding, y + nudge_button_height + nudge_button_padding)); // 162
      track_sample_nudge_down->module = module;
      track_sample_nudge_down->direction = 1;
      addChild(track_sample_nudge_down);
    }
  }

  void onButton(const event::Button &e) override
  {
    Widget::onButton(e);
  } 
};
