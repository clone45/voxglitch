//
// TrackLabelDisplay
//
// Bright orange track name displays that are positioned to
// the right of the track selection buttons
//
struct TrackLabelDisplay : TransparentWidget
{
  GrooveBox *module;
  unsigned int track_number = 0;

  NVGcolor track_background_default = nvgRGBA(68, 77, 140, 255);
  NVGcolor track_background_highlight = nvgRGBA(106, 113, 164, 255);

  TrackLabelDisplay(unsigned int track_number)
  {
    this->track_number = track_number;
    // box.size = Vec(152, 29);
    box.size = Vec(162, 29);
  }

  void onDoubleClick(const event::DoubleClick &e) override
  {
    #ifdef USING_CARDINAL_NOT_RACK
    GrooveBox *module = this->module;
    unsigned int track_number = this->track_number;
    async_dialog_filebrowser(false, NULL, module->samples_root_dir.c_str(), "Load filename", [module, track_number](char* filename) {
      if(filename)
      {
        fileSelected(module, track_number, std::string(filename));
        free(filename);
      }
    });
    #else
    fileSelected(module, this->track_number, module->selectFileVCV());
    #endif
  }

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      module->selectTrack(this->track_number);
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

  void onHover(const event::Hover& e) override {
    TransparentWidget::onHover(e);
    e.consume(this);
  }

  //
  // When using the scroll wheel when hovered over a track label, load either
  // the next or previous sample.  This is a fast way of changing between
  // samples in the same folder.
  //

  /*
  void onHoverScroll(const HoverScrollEvent &e) override
  {
    if(module->shift_key && module->control_key)
    {
      int scroll_distance = (e.scrollDelta.y / 50);

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
          (rack::string::lowercase(system::getExtension(entry)) == ".wav")
        )
        {
          wav_files.push_back(entry);
        }
      }

      // Now that we have a clean list, search for the currently selected
      // wav file.  If we find it (which we should), use the scroll wheel offset
      // to decide which sample to load.
      for(unsigned i=0; i < wav_files.size(); i++)
      {
        std::string filename_in_directory =rack::system::getFilename(wav_files[i]);

        if(filename_in_directory.compare(filename) == 0) // Found it!
        {
          int index = i + scroll_distance; // scroll distance can be negative
          index = clamp(index, 0, wav_files.size() - 1);
          fileSelected(this->module, this->track_number, wav_files[index]);
          break;
        }
      }
      e.consume(this);
    }
    else
    {
      TransparentWidget::onHoverScroll(e);
    }
  }
  */

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

    // Draw background color
    // if(this->focused) draw_focus_overlay(vg);

    // Set up font parameters
    nvgFontSize(vg, 10);
    nvgTextLetterSpacing(vg, 0);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 0xff));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    float wrap_at = 130.0; // Just throw your hands in the air!  And wave them like you just don't 130.0

    // Compute the number of lines that would be drawn
    const char *end = NULL;
    NVGtextRow rows[3];
    unsigned int max_rows = 3;
    unsigned int number_of_lines = nvgTextBreakLines(vg, label.c_str(), NULL, wrap_at, rows, max_rows);

    if(number_of_lines > 1) end = rows[1].end;

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

  void draw(const DrawArgs& args) override
  {

    if(! module->lcd_screen_mode == module->TRACK) return;

    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    //
    // Draw track names
    //
    if(module)
    {
      // Draw track slot background
      nvgBeginPath(vg);
      nvgRect(vg, 0, 0, box.size.x, box.size.y);
      if(module->track_index == this->track_number)
      {
        nvgFillColor(vg, track_background_highlight);
      }
      else
      {
        nvgFillColor(vg, track_background_default);
      }
      nvgFill(vg);

      std::string to_display = module->loaded_filenames[track_number];

      // If the track name is not empty, then display it
      if((to_display != "") && (to_display != "[ empty ]"))
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
      nvgFillColor(vg, track_background_default);
      nvgFill(vg);

      draw_track_label(PLACEHOLDER_TRACK_NAMES[track_number], vg);
    }
    nvgRestore(vg);
  }
};
