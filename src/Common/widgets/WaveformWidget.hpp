struct WaveformWidget : TransparentWidget
{
    Sample *sample;
    // AutobreakStudio *module;
    std::string sample_filename = "";

    bool refresh = true;
    bool *visible;
    float *playback_percentage;

    unsigned int sample_index = 0;
    float draw_area_width = 0.0;

    NVGcolor tab_color_default = nvgRGBA(48, 75, 79, 255);
    NVGcolor tab_color_selected = nvgRGBA(68, 95, 99, 255);
    NVGcolor label_color = nvgRGBA(255, 255, 255, 255);

    // float averages[(int) DRAW_AREA_WIDTH];
    std::vector<float> averages;

    float max_average = 0.0;

    WaveformWidget(Sample *sample, float draw_area_width, float height, bool *visible, float *playback_percentage)
    {
        this->visible = visible;
        this->sample = sample;
        this->playback_percentage = playback_percentage;
        this->draw_area_width = draw_area_width;

        box.size = Vec(draw_area_width, height);
        sample_filename = sample->filename;

        averages.reserve(draw_area_width);

        for (unsigned int i = 0; i < draw_area_width; i++)
        {
            averages[i] = 0.0;
        }
    }

    /* This is not possible, nor necessary
    void setSample(Sample *new_sample)
    {
        this->sample = new_sample;
        refresh = true;
    }
    */

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            const auto vg = args.vg;

            // Save the drawing context to restore later
            nvgSave(vg);

            if (sample && sample->loaded)
            {
                // Compute the values to draw
                if(refresh == true)
                {
                    max_average = 0.0;

                    if (sample->size() > draw_area_width)
                    {
                        for (unsigned int x = 0; x < draw_area_width; x++)
                        {
                            computeAverages(x, sample->size());
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
        
        /*
        
        Here's a bit of a riddle.  In order to decide if this widget should
        be displayed, it's only possible to check a variable in the main
        module class... unless the module class sends in a pointer to a 
        boolean when the widget is first created?
        
        */

        // Sample has changed
        if(sample_filename != sample->filename)
        {
            sample_filename = sample->filename;
            refresh = true;
        }

        if(*visible)
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

        float chunk_size = (float) sample_size / (float) draw_area_width;
        unsigned int chunk_start = (x * chunk_size);
        unsigned int chunk_end = chunk_start +  chunk_size;

        for(unsigned int i=chunk_start; i < chunk_end; i++)
        {
            if(i<sample_size)
            {
                this->sample->read(i, &left, &right);
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
        for(unsigned int x=0; x<draw_area_width; x++)
        {
            averages[x] = (1.0 / max_average) * averages[x];
        }
    }

    void drawWaveform(NVGcontext *vg)
    {
        for (unsigned int x = 0; x < draw_area_width; x++)
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
        nvgRect(vg, x_position, 23.99, 1.0, (height * 22.0 * -1));
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);     

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 23.99, 1.0, (height * 22.0));
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);     
    }

    void drawPositionIndicator(NVGcontext *vg)
    {
        // float x_position = clamp((module->actual_playback_position / sample_size) * DRAW_AREA_WIDTH, (float) 0.0, (float) DRAW_AREA_WIDTH);

        float x_position = clamp(*playback_percentage * draw_area_width, (float) 0.0, (float) draw_area_width);

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 2.0, 6.0, 44.0);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 100));
        nvgFill(vg);
    }

};