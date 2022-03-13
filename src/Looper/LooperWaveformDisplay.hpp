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
  std::deque<float> waveform_array;

  Looper *module;
  std::deque<FadeLine *> fades;
  float playback_position;
  float vertical_position;
  float spacing = 2.4;
  float stroke_width = 3;
  float audio_value = 0.0;

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    if(module)
    {
      audio_value = (module->left_audio + module->right_audio) / 2;
      audio_value = clamp(audio_value, -1.0, 1.0);

      waveform_array.push_front(audio_value);

      if(waveform_array.size() > 75) waveform_array.pop_back();

      for (unsigned int i = 0; i < waveform_array.size(); i++)
      {
        float rect_length = DRAW_AREA_WIDTH * waveform_array[i];
        float rect_x = (DRAW_AREA_WIDTH - rect_length) / 2;

        nvgBeginPath(vg);
        nvgStrokeWidth(vg, stroke_width);
        nvgStrokeColor(vg, nvgRGBA(97, 143, 170, 220));
        nvgMoveTo(vg, rect_x, i * spacing);
        nvgLineTo(vg, rect_x + rect_length, i * spacing);
        nvgStroke(vg);
      }

      // Draw position playback indicator
      playback_position = module->sample_player.getPlaybackPercentage();
      vertical_position = playback_position * 180.0;
      nvgBeginPath(vg);
      nvgStrokeWidth(vg, 2);
      nvgStrokeColor(vg, nvgRGBA(97, 143, 170, 50));
      nvgMoveTo(vg, 0, vertical_position);
      nvgLineTo(vg, DRAW_AREA_WIDTH, vertical_position);
      nvgStroke(vg);
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
