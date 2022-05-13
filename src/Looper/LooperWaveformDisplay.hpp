struct LooperWaveformDisplay : TransparentWidget
{
  Looper *module;
  std::deque<float> waveform_array;

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    // Debugging code for draw area, which often has to be set experimentally
    /*
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
    nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
    nvgFill(vg);
    */

    /*
    if(module)
    {
      waveform_array.push_front(module->left_audio);

      if(waveform_array.size() > 40) waveform_array.pop_back();

      for (unsigned int i = 0; i < waveform_array.size(); i++)
      {
        nvgBeginPath(vg);
        nvgStrokeWidth(vg, 3);
        nvgStrokeColor(vg, nvgRGBA(97, 143, 170, 200));
        nvgMoveTo(vg, (DRAW_AREA_WIDTH / 2), i * 4.3);
        nvgLineTo(vg, (DRAW_AREA_WIDTH / 2) + (DRAW_AREA_WIDTH * waveform_array[i]), i * 4.3);
        nvgStroke(vg);
      }
    }
    */

    nvgRestore(vg);
  }

  void onButton(const event::Button &e) override
  {
    if(isMouseInDrawArea(e.pos))
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        e.consume(this);

        // Open dialog box and prompt user for sample
        const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
#ifdef USING_CARDINAL_NOT_RACK
        Looper *module = this->module;
        async_dialog_filebrowser(false, dir.c_str(), module->loaded_filename.c_str(), [module](char* path) {
            pathSelected(module, path);
        });
#else
        char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));
        pathSelected(module, path);
#endif
      }
    }
  }

  static void pathSelected(Looper *module, char *path)
  {
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
