struct WaveformWidget : TransparentWidget
{
    std::string sample_filename = "";

    bool refresh = true;
    bool draw_container_background = false;
    float width = 0.0;
    float height = 0.0;
    float indicator_width = 6.0;
    float container_padding = 2.0;

    WaveformModel *waveform_modal;

    unsigned int sample_index = 0;

    NVGcolor tab_color_default = nvgRGBA(48, 75, 79, 255);
    NVGcolor tab_color_selected = nvgRGBA(68, 95, 99, 255);
    NVGcolor label_color = nvgRGBA(255, 255, 255, 255);
    NVGcolor container_background_color = nvgRGBA(20, 20, 20, 255);
    NVGcolor indicator_color = nvgRGBA(255, 255, 255, 100);

    std::vector<float> averages;

    float max_average = 0.0;

    // Constructor without position
    WaveformWidget(float width, float height, WaveformModel *waveform_modal)
    {
        this->width = width;
        this->height = height;
        this->waveform_modal = waveform_modal;

        box.size = Vec(width, height);
        sample_filename = waveform_modal->sample->filename;

        averages.reserve((unsigned int)(width - 2 * container_padding));

        for (unsigned int i = 0; i < width - 2 * container_padding; i++)
        {
            averages[i] = 0.0;
        }
    }

    // Constructor with position
    WaveformWidget(float x, float y, float width, float height, WaveformModel *waveform_modal)
    {
        box.pos = Vec(x, y);
        box.size = Vec(width, height);

        this->width = width;
        this->height = height;
        this->waveform_modal = waveform_modal;

        sample_filename = waveform_modal->sample->filename;

        averages.reserve((unsigned int)(width - 2 * container_padding));

        for (unsigned int i = 0; i < width - 2 * container_padding; i++)
        {
            averages[i] = 0.0;
        }
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            const auto vg = args.vg;

            // Save the drawing context to restore later
            nvgSave(vg);

            if (draw_container_background)
                drawContainerBackground(vg);

            if (waveform_modal->sample && waveform_modal->sample->loaded)
            {
                if (refresh)
                {
                    max_average = 0.0;

                    if (waveform_modal->sample->size() > (width - 2 * container_padding))
                    {
                        for (unsigned int x = 0; x < (width - 2 * container_padding); x++)
                        {
                            computeAverages(x, waveform_modal->sample->size());
                        }
                    }

                    normalizeAverages();
                }

                drawWaveform(vg);

                if (waveform_modal->draw_position_indicator)
                    drawPositionIndicator(vg);
                if (waveform_modal->highlight_section)
                    highlightSection(vg);
            }

            nvgRestore(vg);
        }
    }

    void step() override
    {
        TransparentWidget::step();

        // Sample has changed
        if (sample_filename != waveform_modal->sample->filename)
        {
            sample_filename = waveform_modal->sample->filename;
            refresh = true;
        }

        if (waveform_modal->visible)
        {
            this->show();
        }
        else
        {
            this->hide();
        }
    }

    void computeAverages(unsigned int x, unsigned int sample_size)
    {
        float left_sum = 0.0;
        float right_sum = 0.0;
        unsigned int count = 0;

        float left, right;

        float chunk_size = (float)sample_size / (float)(width - 2 * container_padding);
        unsigned int chunk_start = (x * chunk_size);
        unsigned int chunk_end = chunk_start + chunk_size;

        for (unsigned int i = chunk_start; i < chunk_end; i++)
        {
            if (i < sample_size)
            {
                waveform_modal->sample->read(i, &left, &right);
                left_sum += std::abs(left);
                right_sum += std::abs(right);
                count++;
            }
        }

        float left_average = left_sum / (float)count;
        float right_average = right_sum / (float)count;

        averages[x] = (left_average + right_average) / 2.0;

        if (averages[x] > max_average)
            max_average = clamp(averages[x], 0.0, 1.0);

        refresh = false;
    }

    void normalizeAverages()
    {
        for (unsigned int x = 0; x < (width - 2 * container_padding); x++)
        {
            averages[x] = (1.0 / max_average) * averages[x];
        }
    }

    void setIndicatorWidth(float width)
    {
        indicator_width = width;
    }

    void setIndicatorColor(NVGcolor color)
    {
        indicator_color = color;
    }

    void drawWaveform(NVGcontext *vg)
    {
        for (unsigned int x = 0; x < (width - 2 * container_padding); x++)
        {
            drawLine(vg, x);
        }
    }

    void setDrawContainerBackground(bool draw)
    {
        draw_container_background = draw;
    }

    void setContainerBackgroundColor(NVGcolor color)
    {
        container_background_color = color;
    }

    void setContainerPadding(float padding)
    {
        container_padding = padding;
    }

    void drawContainerBackground(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgRect(vg, 0.0, 0.0, width, height);
        nvgFillColor(vg, container_background_color);
        nvgFill(vg);
    }

    void drawLine(NVGcontext *vg, unsigned int x)
    {
        float average_height = averages[x]; // ranges from 0.0 to 1.0
        average_height = clamp(average_height, 0.0, 1.0);
        float line_height = (average_height * (height - 2 * container_padding));

        nvgBeginPath(vg);
        nvgRect(vg, container_padding + x, (height - line_height) / 2.0, 1.0, line_height);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);
    }

    void drawPositionIndicator(NVGcontext *vg)
    {
        float x_position = clamp(waveform_modal->playback_percentage * (width - 2 * container_padding), 0.0f, (float)(width - 2 * container_padding));

        nvgBeginPath(vg);
        nvgRect(vg, container_padding + x_position, container_padding, indicator_width, height - 2 * container_padding);
        nvgFillColor(vg, indicator_color);
        nvgFill(vg);
    }

    void highlightSection(NVGcontext *vg)
    {
        nvgBeginPath(vg);
        nvgRect(vg, container_padding + waveform_modal->highlight_section_x, container_padding, waveform_modal->highlight_section_width, height - 2 * container_padding);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 80));
        nvgFill(vg);
    }

};
