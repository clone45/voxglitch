struct WaveformWidget : TransparentWidget
{
    Sample *sample;
    AutobreakStudio *module;
    std::string sample_filename = "";

    bool refresh = true;
    unsigned int sample_index = 0;

    NVGcolor tab_color_default = nvgRGBA(48, 75, 79, 255);
    NVGcolor tab_color_selected = nvgRGBA(68, 95, 99, 255);
    NVGcolor label_color = nvgRGBA(255, 255, 255, 255);

    float averages[(int) DRAW_AREA_WIDTH];
    float max_average = 0.0;

    WaveformWidget(AutobreakStudio *module, unsigned int index)
    {
        this->module = module;
        this->sample_index = index;

        box.size = Vec(DRAW_AREA_WIDTH, LCD_TABS_HEIGHT);

        for (unsigned int i = 0; i < DRAW_AREA_WIDTH; i++)
        {
            averages[i] = 0.0;
        }
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if(module->samples[sample_index].filename != this->sample_filename)
        {
            this->sample_filename = module->samples[sample_index].filename;
            refresh = true;
        }

        if (layer == 1)
        {
            const auto vg = args.vg;

            // Save the drawing context to restore later
            nvgSave(vg);

            if (module && module->samples[sample_index].loaded)
            {
                // Compute the values to output
                unsigned int sample_size = module->samples[sample_index].size();

                //
                // Compute the values to draw
                //
                if(refresh == true)
                {
                    max_average = 0.0;

                    if (sample_size > DRAW_AREA_WIDTH)
                    {
                        for (unsigned int x = 0; x < DRAW_AREA_WIDTH; x++)
                        {
                            computeAverages(x, sample_size);
                        }
                    }

                    normalizeAverages();
                }

                drawWaveform(vg);
                drawPositionIndicator(vg, sample_size);
            }

            nvgRestore(vg);
        }
    }

    void step() override 
    {
        TransparentWidget::step();

        if(this->sample_index == module->selected_sample_slot)
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

        float chunk_size = (float) sample_size / (float) DRAW_AREA_WIDTH;
        unsigned int chunk_start = (x * chunk_size);
        unsigned int chunk_end = chunk_start +  chunk_size;

        for(unsigned int i=chunk_start; i < chunk_end; i++)
        {
            if(i<sample_size)
            {
                module->samples[this->sample_index].read(i, &left, &right);
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
        for(unsigned int x=0; x<DRAW_AREA_WIDTH; x++)
        {
            averages[x] = (1.0 / max_average) * averages[x];
        }
    }

    void drawWaveform(NVGcontext *vg)
    {
        for (unsigned int x = 0; x < DRAW_AREA_WIDTH; x++)
        {
            drawLine(vg, x);
        }
    }

    void drawLine(NVGcontext *vg, unsigned int x)
    {
        float x_position = x;
        float height = averages[x];

        height = clamp(height, 0.0, 1.0);

        /* TODO: Figure this out
        float y = (44.0 - (44.0 * height)) / 2.0;

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 0.0, y, height * 44.0);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);
        */

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 23.99, 1.0, (height * 22.0 * -1));
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);     


        nvgBeginPath(vg);
        nvgRect(vg, x_position, 23.99, 1.0, (height * 22.0));
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);     
    }

    void drawPositionIndicator(NVGcontext *vg, unsigned int sample_size)
    {
        float x_position = clamp((module->actual_playback_position / sample_size) * DRAW_AREA_WIDTH, (float) 0.0, (float) DRAW_AREA_WIDTH);

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 2.0, 6.0, 44.0);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 100));
        nvgFill(vg);
    }

};