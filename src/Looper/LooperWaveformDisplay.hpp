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
  unsigned int update_countdown = 0;

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

    if(module)
    {
      if(update_countdown == 0)
      {
        for (unsigned int i = 0; i < fades.size(); i++) {
          fades[i]->fadeOut();
        }
        float playback_position = module->sample_player.getPlaybackPercentage();
        float vertical_position = playback_position * 180.0;

        fades.push_front(new FadeLine(vertical_position));
        if(fades.size() > 10) fades.pop_back();
        update_countdown = 10;
      }
      else
      {
        //  only draw the
        update_countdown --;
      }

      for (unsigned int i = 0; i < fades.size(); i++)
      {
        nvgBeginPath(vg);
        nvgStrokeWidth(vg, 5);
        if(i == 0) nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 255));
        if(i != 0) nvgStrokeColor(vg, nvgRGBA(255, 255, 255, fades[i]->fade * 20));
        nvgMoveTo(vg, 0, fades[i]->y);
        nvgLineTo(vg, DRAW_AREA_WIDTH, fades[i]->y);
        nvgStroke(vg);

        // fades[i]->fadeOut();
      }
    }

    nvgRestore(vg);


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
