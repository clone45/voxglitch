struct FadeLine
{
  float y = 0;
  float fade = 1.0;
  float fade_rate = .1;

  FadeLine(float vertical_position)
  {
    y = vertical_position;
  }

  void fadeOut()
  {
    if(fade > 0)
    {
      fade -= fade_rate;
    }
    else
    {
      fade = 0;
    }
  }
};

struct LooperWaveformDisplay : TransparentWidget
{
  Looper *module;
  std::deque<FadeLine *> fades;
  float playback_position;
  float vertical_position;
  float spacing = 2.4;
  float stroke_width = 3;
  float audio_value = 0.0;

  void draw(const DrawArgs &args) override
  {
    /*
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(args.vg, nvgRGBA(120, 20, 20, 100));
    nvgFill(args.vg);
    */
  }

  void onDoubleClick(const DoubleClickEvent &e) override
  {
    // Open dialog box and prompt user for sample
    const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
    #ifdef USING_CARDINAL_NOT_RACK
      Looper *module = this->module;
      async_dialog_filebrowser(false, NULL, dir.c_str(), module->loaded_filename.c_str(), [module](char* path) {
        pathSelected(module, path);
      });
    #else
      char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));
      pathSelected(module, path);
    #endif
  }

  static void pathSelected(Looper *module, char *path)
  {
    if (path)
    {
      module->root_dir = std::string(path);
      module->sample_player.loadSample(std::string(path));
      module->loaded_filename = module->sample_player.getFilename();
      free(path);
    }
  }

  /*
  void onButton(const event::Button &e) override
  {
    if(isMouseInDrawArea(e.pos))
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        e.consume(this);

        // Open dialog box and prompt user for sample
    		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
    		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

    		if (path)
    		{
          module->root_dir = std::string(path);
    			module->sample_player.loadSample(std::string(path));
          module->loaded_filename = module->sample_player.getFilename();
    			free(path);

          // Redundant and dangerous code
          // module->sample_player.loadSample(std::string(path));
    		}

      }
    }
  }
  */

  bool isMouseInDrawArea(Vec position)
  {
    if(position.x < 0) return(false);
    if(position.y < 0) return(false);
    if(position.x >= DRAW_AREA_WIDTH) return(false);
    if(position.y >= DRAW_AREA_HEIGHT) return(false);
    return(true);
  }

  void onDragMove(const event::DragMove &e) override
  {
    TransparentWidget::onDragMove(e);
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
  }

  void step() override {
    TransparentWidget::step();
  }

};
