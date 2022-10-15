struct WaveformWidget : TransparentWidget
{
    // AutobreakStudio *module;
    std::string sample_filename = "";

    bool refresh = true;
    float width = 0.0;
    float height = 0.0;

    // Sample *sample;
    // bool *visible;
    // float *playback_percentage;
    WaveformModel *waveform_modal;

    unsigned int sample_index = 0;

    NVGcolor tab_color_default = nvgRGBA(48, 75, 79, 255);
    NVGcolor tab_color_selected = nvgRGBA(68, 95, 99, 255);
    NVGcolor label_color = nvgRGBA(255, 255, 255, 255);

    // float averages[(int) DRAW_AREA_WIDTH];
    std::vector<float> averages;

    float max_average = 0.0;

    WaveformWidget(float width, float height, WaveformModel *waveform_modal)
    {
        this->width = width;
        this->height = height;
        this->waveform_modal = waveform_modal;

        box.size = Vec(width, height);
        sample_filename = waveform_modal->sample->filename;

        averages.reserve((unsigned int) width);

        for (unsigned int i = 0; i < width; i++)
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

            if (waveform_modal->sample && waveform_modal->sample->loaded)
            {
                // Compute the values to draw
                if(refresh == true)
                {
                    max_average = 0.0;

                    if (waveform_modal->sample->size() > this->width)
                    {
                        for (unsigned int x = 0; x < this->width; x++)
                        {
                            computeAverages(x, waveform_modal->sample->size());
                        }
                    }

                    normalizeAverages();
                }

                drawWaveform(vg);
                drawPositionIndicator(vg);
            }

            nvgRestore(vg);
        }
    }

    void step() override 
    {
        TransparentWidget::step();

        // Sample has changed
        if(sample_filename != waveform_modal->sample->filename)
        {
            sample_filename = waveform_modal->sample->filename;
            refresh = true;
        }

        if(waveform_modal->visible)
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
        // Get average height of line at index
        float left_sum = 0.0;
        float right_sum = 0.0;
        unsigned int count = 0;

        float left;
        float right;

        float chunk_size = (float) sample_size / (float) width;
        unsigned int chunk_start = (x * chunk_size);
        unsigned int chunk_end = chunk_start +  chunk_size;

        for(unsigned int i=chunk_start; i < chunk_end; i++)
        {
            if(i<sample_size)
            {
                waveform_modal->sample->read(i, &left, &right);
                left_sum = left_sum + std::abs(left);
                right_sum = right_sum + std::abs(right);
                count++;
            }
        }

        float left_average = left_sum / (float) count;
        float right_average = right_sum / (float) count;

        averages[x] = (left_average + right_average) / 2.0;

        if(averages[x] > max_average)  max_average = clamp(averages[x], 0.0, 1.0);

        refresh = false;
    }

    void normalizeAverages()
    {
        for(unsigned int x = 0; x < this->width; x++)
        {
            averages[x] = (1.0 / max_average) * averages[x];
        }
    }

    void drawWaveform(NVGcontext *vg)
    {
        for (unsigned int x = 0; x < this->width; x++)
        {
            drawLine(vg, x);
        }
    }

    void drawLine(NVGcontext *vg, unsigned int x)
    {
        float x_position = x;
        float height = averages[x];

        height = clamp(height, 0.0, 1.0);

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 23.99, 1.0, (height * 22.0 * -1));  // todo: compute this instead of hard coded
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);     

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 23.99, 1.0, (height * 22.0));  // todo: compute this instead of hard coded
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);     
    }

    void drawPositionIndicator(NVGcontext *vg)
    {
        // float x_position = clamp((module->actual_playback_position / sample_size) * DRAW_AREA_WIDTH, (float) 0.0, (float) DRAW_AREA_WIDTH);

        float x_position = clamp(waveform_modal->playback_percentage * width, (float) 0.0, (float) width);

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 2.0, 6.0, 44.0); // todo: compute this instead of hard coded
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 100));
        nvgFill(vg);
    }

};