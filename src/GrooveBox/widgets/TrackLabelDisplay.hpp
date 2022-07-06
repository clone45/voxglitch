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

  TrackLabelDisplay(unsigned int track_number)
  {
    this->track_number = track_number;
    box.size = Vec(152, 29);
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

  static void fileSelected(GrooveBox *module, unsigned int track_number, std::string filename)
	{
		if (filename != "")
		{
			module->sample_players[track_number].loadSample(filename);
      module->loaded_filenames[track_number] = module->sample_players[track_number].getFilename();
			module->setRoot(filename);
		}
	}


  void onButton(const event::Button &e) override
  {
    TransparentWidget::onButton(e);
    e.consume(this);
  }

  void draw_track_label(std::string label, NVGcontext *vg)
  {
    float text_left_margin = 6;

    nvgFontSize(vg, 10);
    nvgTextLetterSpacing(vg, 0);
    nvgFillColor(vg, nvgRGBA(255, 215, 20, 0xff));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    float wrap_at = 130.0; // Just throw your hands in the air!  And wave them like you just don't 130.0

    const char *end = NULL;
    NVGtextRow rows[3];
    unsigned int max_rows = 3;
    unsigned int number_of_lines = nvgTextBreakLines(vg, label.c_str(), NULL, wrap_at, rows, max_rows);

    if(number_of_lines > 1) end = rows[1].end;

    float bounds[4];
    nvgTextBoxBounds(vg, text_left_margin, 10, wrap_at, label.c_str(), end, bounds);

    float textHeight = bounds[3];
    nvgTextBox(vg, text_left_margin, (box.size.y / 2.0f) - (textHeight / 2.0f) + 8, wrap_at, label.c_str(), end);
  }

  void draw_track_mute_overlay(NVGcontext *vg)
  {
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 162));
    nvgFill(vg);
  }

  void draw(const DrawArgs& args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    // Draw dark background
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(20, 20, 20, 255));
    nvgFill(vg);

    //
    // Draw track names
    //
    if(module)
    {
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
      draw_track_label(PLACEHOLDER_TRACK_NAMES[track_number], vg);
    }
    nvgRestore(vg);
  }
};
