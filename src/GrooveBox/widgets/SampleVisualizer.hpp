struct SampleVisualizerWidget : TransparentWidget
{
  GrooveBox *module;

  float width = 0;
  float height = 0;
  float mid_height = 0;

  unsigned int columns = 390;
  float stroke_width = 1;

  SampleVisualizerWidget(float width, float height)
  {
    this->width = width;
    this->height = height;
    this->mid_height = height / 2.0;
    this->box.size = Vec(width, height);
  }

  void drawLayer(const DrawArgs& args, int layer) override
  {
  	if (layer == 1)
    {
      TransparentWidget::draw(args);
      const auto vg = args.vg;

      if(module)
      {
        nvgSave(vg);

        if(module->lcd_screen_mode == module->SAMPLE)
        {
          Sample *active_sample = & module->selected_track->sample_player->sample;

          unsigned int sample_size = active_sample->size();
          unsigned int index_offset = sample_size / columns;

          // Draw waveform
          for (unsigned int i = 0; i < columns; i++)
          {
            unsigned int sample_index = index_offset * i;

            float left_audio = 0;
            float right_audio = 0;

            active_sample->read(sample_index, &left_audio, &right_audio);

            left_audio = clamp(left_audio * 0.5f, -0.5, 0.5);

            float rect_height = (height * left_audio);
            float rect_x = (width / columns) * i;

            nvgBeginPath(vg);
            nvgStrokeWidth(vg, stroke_width);
            nvgStrokeColor(vg, nvgRGB(240, 240, 240));
            nvgMoveTo(vg, rect_x, mid_height);
            nvgLineTo(vg, rect_x, mid_height + rect_height);
            nvgStroke(vg);
          }

          // Draw range rectangle
          float sample_start = module->selected_track->sample_playback_settings[module->visualizer_step].sample_start;
          float sample_end = module->selected_track->sample_playback_settings[module->visualizer_step].sample_end;

          nvgBeginPath(vg);

          nvgRect(vg, sample_start * width, 0, (sample_end * width) - (sample_start * width), height);

          if(sample_start <= sample_end){
            // Friendly blue highlight
            nvgFillColor(vg, nvgRGBA(97, 143, 170, 50));
          }
          else {
            // Unhappy red highlight
            nvgFillColor(vg, nvgRGBA(143, 90, 90, 80));
          }

          nvgFill(vg);
        }

        nvgRestore(vg);
      }
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
    e.consume(this);
  }

  void step() override {
    TransparentWidget::step();
  }
};
