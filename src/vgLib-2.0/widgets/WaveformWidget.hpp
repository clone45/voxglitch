struct WaveformWidget : TransparentWidget
{
    std::string sample_filename = "";

    bool refresh = true;
    bool draw_container_background = false;
    float width = 0.0;
    float height = 0.0;
    float indicator_width = 2.0;
    unsigned int initial_sample_position = 0;

    // Replace single padding with individual paddings
    float container_padding_top = 2.0;
    float container_padding_right = 2.0;
    float container_padding_bottom = 2.0;
    float container_padding_left = 2.0;

    WaveformModel *waveform_modal;

    unsigned int sample_index = 0;

    NVGcolor tab_color_default = nvgRGBA(48, 75, 79, 255);
    NVGcolor tab_color_selected = nvgRGBA(68, 95, 99, 255);
    NVGcolor label_color = nvgRGBA(255, 255, 255, 255);
    NVGcolor container_background_color = nvgRGBA(20, 20, 20, 255);
    NVGcolor indicator_color = nvgRGBA(255, 255, 255, 100);

    std::vector<float> averages;

    float max_average = 0.0;

    bool scrubber_hover = false;
    bool scrubber_dragging = false;
    float scrubber_hit_zone = 5.0f;
    
    Vec drag_position;
    float initial_percentage = 0.0f;
    float cumulative_drag_offset = 0.0f;

    // Constructor without position
    WaveformWidget(float width, float height, WaveformModel *waveform_modal)
    {
        this->width = width;
        this->height = height;
        this->waveform_modal = waveform_modal;

        box.size = Vec(width, height);
        sample_filename = waveform_modal->sample->filename;

        // Use horizontal padding for width calculation
        averages.reserve((unsigned int)(width - (container_padding_left + container_padding_right)));

        for (unsigned int i = 0; i < width - (container_padding_left + container_padding_right); i++)
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

        averages.reserve((unsigned int)(width - (container_padding_left + container_padding_right)));

        for (unsigned int i = 0; i < width - (container_padding_left + container_padding_right); i++)
        {
            averages[i] = 0.0;
        }
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            const auto vg = args.vg;

            nvgSave(vg);

            if (draw_container_background)
                drawContainerBackground(vg);

            if (waveform_modal->sample && waveform_modal->sample->loaded)
            {
                if (refresh)
                {
                    max_average = 0.0;

                    if (waveform_modal->sample->size() > (width - (container_padding_left + container_padding_right)))
                    {
                        for (unsigned int x = 0; x < (width - (container_padding_left + container_padding_right)); x++)
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

                drawMarkers(vg);
            }

            nvgRestore(vg);
        }
    }

    void step() override
    {
        TransparentWidget::step();

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

        float chunk_size = (float)sample_size / (float)(width - (container_padding_left + container_padding_right));
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
        for (unsigned int x = 0; x < (width - (container_padding_left + container_padding_right)); x++)
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
        for (unsigned int x = 0; x < (width - (container_padding_left + container_padding_right)); x++)
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

    // New padding setters
    void setContainerPadding(float padding)
    {
        container_padding_top = padding;
        container_padding_right = padding;
        container_padding_bottom = padding;
        container_padding_left = padding;
    }

    void setContainerPadding(float vertical, float horizontal)
    {
        container_padding_top = vertical;
        container_padding_bottom = vertical;
        container_padding_left = horizontal;
        container_padding_right = horizontal;
    }

    void setContainerPadding(float top, float right, float bottom, float left)
    {
        container_padding_top = top;
        container_padding_right = right;
        container_padding_bottom = bottom;
        container_padding_left = left;
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
        float average_height = averages[x];
        average_height = clamp(average_height, 0.0, 1.0);
        float line_height = (average_height * (height - (container_padding_top + container_padding_bottom)));

        nvgBeginPath(vg);
        nvgRect(vg, 
            container_padding_left + x, 
            (height - line_height) / 2.0, 
            1.0, 
            line_height
        );
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
        nvgFill(vg);
    }

    void drawPositionIndicator(NVGcontext *vg)
    {
        if (!waveform_modal || !waveform_modal->sample) return;

        float drawable_width = width - (container_padding_left + container_padding_right);
        
        // Convert sample position to x coordinate
        float relative_pos = static_cast<float>(waveform_modal->playhead_position) / 
            static_cast<float>(waveform_modal->sample->size());
        float x_position = container_padding_left + (relative_pos * drawable_width);

        // Clamp to drawable area
        x_position = rack::math::clamp(
            x_position,
            container_padding_left,
            width - container_padding_right
        );

        nvgBeginPath(vg);
        nvgRect(vg, 
            x_position - (indicator_width / 2.0f),  // Center the indicator 
            container_padding_top, 
            indicator_width, 
            height - (container_padding_top + container_padding_bottom)
        );
        nvgFillColor(vg, indicator_color);
        nvgFill(vg);
    }

    void highlightSection(NVGcontext *vg)
    {
        if (!waveform_modal || !waveform_modal->sample) return;

        float drawable_width = width - (container_padding_left + container_padding_right);
        float sample_size = static_cast<float>(waveform_modal->sample->size());

        // Convert sample positions to x coordinates
        float start_x = container_padding_left + 
            (static_cast<float>(waveform_modal->highlight_section_start) / sample_size * drawable_width);
        float section_width = static_cast<float>(waveform_modal->highlight_section_length) / 
            sample_size * drawable_width;

        nvgBeginPath(vg);
        nvgRect(vg, 
            start_x,
            container_padding_top, 
            section_width, 
            height - (container_padding_top + container_padding_bottom)
        );
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 80));
        nvgFill(vg);
    }

    void drawMarkers(NVGcontext *vg)
    {
        if (!waveform_modal || !waveform_modal->sample) return;

        float drawable_width = width - (container_padding_left + container_padding_right);
        float sample_size = static_cast<float>(waveform_modal->sample->size());

        nvgBeginPath(vg);
        for (float marker_pos : waveform_modal->marker_positions)
        {
            float relative_pos = marker_pos / sample_size;
            float x_position = container_padding_left + (relative_pos * drawable_width);

            x_position = rack::math::clamp(
                x_position,
                container_padding_left,
                width - container_padding_right
            );

            nvgMoveTo(vg, x_position, container_padding_top);
            nvgLineTo(vg, x_position, height - container_padding_bottom);
        }

        nvgStrokeColor(vg, nvgRGBA(200, 200, 200, 100));
        nvgStrokeWidth(vg, 1.0f);
        nvgStroke(vg);
    }

    void onButton(const event::Button &e) override
    {
        TransparentWidget::onButton(e);
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            if (e.action == GLFW_PRESS && waveform_modal && waveform_modal->sample) {
                scrubber_dragging = true;
                waveform_modal->scrubber_dragging = true;
                drag_position = e.pos;
                e.consume(this);
            }
            else if (e.action == GLFW_RELEASE) {
                scrubber_dragging = false;
                waveform_modal->scrubber_dragging = false;
            }
        }
    }

    void onDragMove(const event::DragMove& e) override {
        if (scrubber_dragging) {
            float zoom = getAbsoluteZoom();
            float current_x = drag_position.x + e.mouseDelta.x / zoom;
            float drawable_width = width - (container_padding_left + container_padding_right);
            float relative_x = (current_x - container_padding_left) / drawable_width;
            relative_x = rack::math::clamp(relative_x, 0.0f, 1.0f);
            
            unsigned int new_position = static_cast<unsigned int>(
                relative_x * waveform_modal->sample->size()
            );

            waveform_modal->onDragPlayhead(new_position);
            drag_position.x = current_x;
            e.consume(this);
        }
    }

    void onHover(const event::Hover &e) override
    {
        e.consume(this);

        if (!waveform_modal->sample || !waveform_modal->sample->loaded) {
            return;
        }

        float scrubber_x = container_padding_left + 
            (waveform_modal->playback_percentage * (width - (container_padding_left + container_padding_right)));
        
        if (std::abs(e.pos.x - scrubber_x) < scrubber_hit_zone) {
            glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
            scrubber_hover = true;
        } else {
            if (scrubber_hover) {
                glfwSetCursor(APP->window->win, NULL);
                scrubber_hover = false;
            }
        }
    }

    void onLeave(const event::Leave &e) override
    {
        TransparentWidget::onLeave(e);
        glfwSetCursor(APP->window->win, NULL);
    }
};