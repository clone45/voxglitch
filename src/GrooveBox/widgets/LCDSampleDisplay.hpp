struct LCDSampleDisplay : LCDDisplay
{
  GrooveBox *module;

  // float width = 0;
  // float height = 0;
  float mid_height = 0;

  unsigned int columns = 390;
  float stroke_width = 1;

  LCDSampleDisplay(GrooveBox *module)
  {
    this->module = module;

    box.pos.x = this->box_pos_x;
    box.pos.y = this->box_pos_y;
    box.size = Vec(this->box_width, this->box_height);

    // this->width = width;
    // this->height = height;
    this->mid_height = box.size.y / 2.0;
    // this->box.size = Vec(width, height);
  }

  void drawLayer(const DrawArgs &args, int layer) override
  {
    if (layer == 1)
    {
      const auto vg = args.vg;

      if (module)
      {
        nvgSave(vg);

        if (module->lcd_screen_mode == module->SAMPLE)
        {
          Sample *active_sample = &module->selected_track->sample_player->sample;

          unsigned int sample_size = active_sample->size();
          unsigned int index_offset = sample_size / columns;

          //
          // Draw waveform
          //

          for (unsigned int i = 0; i < columns; i++)
          {
            unsigned int sample_index = index_offset * i;

            float left_audio = 0;
            float right_audio = 0;

            active_sample->read(sample_index, &left_audio, &right_audio);

            left_audio = clamp(left_audio * 0.5f, -0.5, 0.5);

            float rect_height = ((box.size.y - ( 2.0 * display_padding)) * left_audio);
            float rect_x = display_padding + (((box.size.x - (2.0 * display_padding)) / columns) * i);

            nvgBeginPath(vg);
            nvgStrokeWidth(vg, stroke_width);
            nvgStrokeColor(vg, nvgRGB(255, 255, 255));
            nvgMoveTo(vg, rect_x, mid_height);
            nvgLineTo(vg, rect_x, mid_height + rect_height);
            nvgStroke(vg);
          }

          //
          // Draw range rectangle
          //

          // float sample_start = module->selected_track->parameter_lock_settings[module->visualizer_step].sample_start;
          // float sample_end = module->selected_track->parameter_lock_settings[module->visualizer_step].sample_end;

          float sample_start = module->selected_track->getParameter(SAMPLE_START, module->visualizer_step);
          float sample_end = module->selected_track->getParameter(SAMPLE_END, module->visualizer_step);


          nvgBeginPath(vg);

          float waveform_width = box.size.x - (2.0 * display_padding);

          float draw_from_x = display_padding + (sample_start * waveform_width);

          nvgRect(vg, draw_from_x, display_padding, display_padding + (sample_end * waveform_width) - draw_from_x, box.size.y - (2.0 * display_padding));

          if (sample_start <= sample_end)
          {
            // Friendly blue highlight
            nvgFillColor(vg, LCDColorScheme::getStrongHighlightOverlay());
          }
          else
          {
            // Unhappy red highlight
            nvgFillColor(vg, nvgRGBA(143, 90, 90, 80));
          }

          nvgFill(vg);
        }

        nvgRestore(vg);
      }
    }
    Widget::drawLayer(args, layer);
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
    e.consume(this);
  }

  void step() override
  {
    TransparentWidget::step();
  }
};
