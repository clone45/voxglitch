//
// TrackSampleNudge
//
struct TrackSampleNudge : TransparentWidget
{
  GrooveBox *module;
  unsigned int track_number = 0;

  NVGcolor track_background_default = nvgRGBA(68, 77, 140, 255);
  NVGcolor track_background_highlight = nvgRGBA(106, 113, 164, 255);

  int direction = 1;

  TrackSampleNudge(unsigned int track_number)
  {
    this->track_number = track_number;
    box.size = Vec(20, (29.0 / 2.0) - 0.5);
  }

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      module->selectTrack(this->track_number);

      std::string path = module->sample_players[track_number].getPath();
      std::string directory = rack::system::getDirectory(path);
      std::string filename = module->sample_players[track_number].getFilename();

      std::vector<std::string> directory_list = system::getEntries(directory);

      // Sort the vector.  This is in response to a user who's samples were being
      // loaded out of order.  I think it's a mac thing.
      sort(directory_list.begin(), directory_list.end());

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
      // wav file.  If we find it (which we should), find the next sample.
      for(unsigned i=0; i < wav_files.size(); i++)
      {
        std::string filename_in_directory =rack::system::getFilename(wav_files[i]);

        if(filename_in_directory.compare(filename) == 0) // Found it!
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

  void onHover(const event::Hover& e) override {
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

  void draw(const DrawArgs& args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    //
    // Draw nudge button
    //
    if(module)
    {
      if(! module->lcd_screen_mode == module->TRACK)
      {
        nvgRestore(vg);
        return;
      } 

      // Draw nudge rectangle background
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

      drawArrow(vg, direction);

    }
    //
    // Nudge button
    //
    else
    {
      // Draw nudge rectangle background
      nvgBeginPath(vg);
      nvgRect(vg, 0, 0, box.size.x, box.size.y);
      nvgFillColor(vg, track_background_default);
      nvgFill(vg);

      drawArrow(vg, direction);
    }
    nvgRestore(vg);
  }

  void drawArrow(NVGcontext *vg, int direction)
  {
    nvgFontSize(vg, 10);

    nvgFillColor(vg, nvgRGBA(255, 255, 255, 100));
    if(module && module->track_index == this->track_number) nvgFillColor(vg, nvgRGBA(255, 255, 255, 0xff));
    
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

    char *end = NULL;

    // Plot character
    if(direction < 0)
    {
      nvgText(vg, 10.0, 6.0, "▲", end);
    }
    else
    {
      nvgText(vg, 10.0, 7.0, "▼", end);
    }
  }
};
