struct RatchetVisualizerWidget : TransparentWidget
{
  GrooveBox *module;

  float lcd_width = 133.741 * 2.952756;
  float lcd_height = 49.672 * 2.952756;
  float mid_height = lcd_height / 2.0;
  unsigned int columns = 390;
  float stroke_width = 1;
  float padding = 4;
  float cell_width = 22;
  float cell_height = 12;
  float highlight = 0;

  float lcd_padding_top = (lcd_height - ((8 * cell_height) + (padding * 7)))/ 2.0;
  float lcd_padding_left = lcd_padding_top;

  void drawLayer(const DrawArgs& args, int layer) override
  {
  	if (layer == 1)
    {
      TransparentWidget::draw(args);
      const auto vg = args.vg;

      if(module && module->show_ratchet_visualizer > 0)
      {
        nvgSave(vg);

        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, lcd_width, lcd_height);
        nvgFillColor(vg, nvgRGB(0, 0, 0));
        nvgFill(vg);

        // Determine the selected ratchet pattern
        float ratchet_float_value = module->selected_track->sample_playback_settings[module->visualizer_step].ratchet;
        unsigned int ratchet_pattern_index = ratchet_float_value * NUMBER_OF_RATCHET_PATTERNS;

        for(unsigned int i=0; i<NUMBER_OF_RATCHET_PATTERNS; i++)
        {
          unsigned int column = i / (NUMBER_OF_RATCHET_PATTERNS / 2);
          float x_offset = (column * (lcd_width / 2.0)) - (padding * column);

          for(unsigned int ratchet_array_index = 0; ratchet_array_index < 7; ratchet_array_index++)
          {
            float x = lcd_padding_top + ((ratchet_array_index * cell_width) + x_offset + (padding * ratchet_array_index));
            float y = lcd_padding_left + ((cell_height + padding) * (i % (NUMBER_OF_RATCHET_PATTERNS / 2)));

            highlight = (ratchet_pattern_index == i) ? 70 : 0;

            nvgBeginPath(vg);
            nvgRect(vg, x, y, cell_width, cell_height);

            bool ratchet_array_boolean = ratchet_patterns[i][ratchet_array_index];

            if(ratchet_array_boolean){
              nvgFillColor(vg, nvgRGBA(97, 143, 170, 100 + highlight)); // Friendly blue highlight
            }
            else {
              nvgFillColor(vg, nvgRGBA(90, 90, 90, 80 + highlight)); // Grey color
            }
            nvgFill(vg);
          }
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
