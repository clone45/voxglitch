struct XYDisplay : VoxglitchWidget
{
    XY *module;
    bool dragging = false;
    std::vector<Vec> fading_rectangles;
    NVGcolor rectangle_colors[30];
    float display_timer = 0.0;

    unsigned int number_of_rectangles = 10;
    unsigned int fade_breakpoint = 30;

    XYDisplay(XY *module) : VoxglitchWidget()
    {
        this->module = module;
        box.size = Vec(DRAW_AREA_WIDTH_PT, DRAW_AREA_HEIGHT_PT);

        for (unsigned int i = 0; i < number_of_rectangles; i++)
        {
            if (i == (number_of_rectangles - 1))
            {
                rectangle_colors[i] = nvgRGBA(255, 255, 255, 255);
            }
            else
            {
                rectangle_colors[i] = nvgRGBA(255, 255, 255, fade_breakpoint - (i * (fade_breakpoint / number_of_rectangles)));
            }
        }
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            VoxglitchWidget::draw(args);
            const auto vg = args.vg;

            if (module)
            {
                float now_x = this->module->drag_position.x;
                float now_y = this->module->drag_position.y - DRAW_AREA_HEIGHT_PT;
                float drag_y = this->module->drag_position.y;

                nvgSave(vg);

                // draw x,y lines, just because I think they look cool

                nvgBeginPath(vg);
                nvgStrokeWidth(vg, 0.5f);
                nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
                nvgMoveTo(vg, now_x, 0);
                nvgLineTo(vg, now_x, DRAW_AREA_HEIGHT_PT);
                nvgStroke(vg);

                nvgBeginPath(vg);
                nvgStrokeWidth(vg, 0.5f);
                nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
                nvgMoveTo(vg, 0, drag_y);
                nvgLineTo(vg, DRAW_AREA_WIDTH_PT, drag_y);
                nvgStroke(vg);

                display_timer += 1.0f / APP->window->getLastFrameDuration();

                if (display_timer >= 100.0)
                {
                    fading_rectangles.push_back(Vec(now_x, now_y));

                    if (fading_rectangles.size() > number_of_rectangles)
                    {
                        fading_rectangles.erase(fading_rectangles.begin());
                    }

                    display_timer = 0;
                }

                int i = 0;

                for (auto position : fading_rectangles)
                {
                    float x = position.x;
                    float y = position.y;

                    // draw thing
                    nvgBeginPath(vg);
                    nvgRect(vg, 0, DRAW_AREA_WIDTH_PT, x, y);
                    nvgFillColor(vg, rectangle_colors[i++]);
                    nvgFill(vg);
                }

                // FOR TESTING: Draw test rectable to see bounds of box
                /*
                nvgBeginPath(vg);
                nvgStrokeColor(vg, nvgRGB(100, 120, 255));
                nvgStrokeWidth(vg, 0.2f);
                nvgFillColor(vg, nvgRGBA(0,0,0,0));
                nvgRect(vg, 0, 0, DRAW_AREA_WIDTH_PT, DRAW_AREA_HEIGHT_PT);
                nvgStroke(vg);
                */

                nvgRestore(vg);
            }
            else
            {
                nvgSave(vg);

                // draw preview for module browser

                // vertical line
                nvgBeginPath(vg);
                nvgStrokeWidth(vg, 0.5f);
                nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
                nvgMoveTo(vg, 120, 0);
                nvgLineTo(vg, 120, DRAW_AREA_HEIGHT_PT);
                nvgStroke(vg);

                // horizontal line
                nvgBeginPath(vg);
                nvgStrokeWidth(vg, 0.5f);
                nvgStrokeColor(vg, nvgRGBA(0xdd, 0xdd, 0xdd, 255));
                nvgMoveTo(vg, 0, 150);
                nvgLineTo(vg, DRAW_AREA_WIDTH_PT, 150);
                nvgStroke(vg);

                // filled rectangle
                nvgBeginPath(vg);
                nvgRect(vg, 0, DRAW_AREA_WIDTH_PT, 120, -130);
                nvgFillColor(vg, rectangle_colors[29]);
                nvgFill(vg);

                nvgRestore(vg);
            }
        }
    }

    Vec clampToDrawArea(Vec location)
    {
        float x = clamp(location.x, 0.0f, DRAW_AREA_WIDTH_PT);
        float y = clamp(location.y, 0.0f, DRAW_AREA_HEIGHT_PT);
        return (Vec(x, y));
    }

    void onButton(const event::Button &e) override
    {
        e.consume(this);
        this->module->drag_position = this->clampToDrawArea(e.pos);

        //
        // Punch recording mode NOT enabled
        //
        if (this->module->get_punch_switch_value() == 0)
        {
            // Press left Mouse Button to start recording
            if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
                this->module->start_recording();

            // Release left mouse button to stop recording and start playback
            if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
                this->module->start_playback();
        }
        //
        // Punch recording mode enabled
        //
        else
        {
            // Press left Mouse Button to start punch recording
            if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
                this->module->start_punch_recording();

            // Release left mouse button to stop recording and continue playback
            if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
                this->module->continue_playback();
        }
    }

    void onDragStart(const event::DragStart &e) override
    {
        VoxglitchWidget::onDragStart(e);
        dragging = true;
    }

    void onDragEnd(const event::DragEnd &e) override
    {
        VoxglitchWidget::onDragEnd(e);
        dragging = false;
    }

    void onDragMove(const event::DragMove &e) override
    {
        VoxglitchWidget::onDragMove(e);
        float zoom = getAbsoluteZoom();
        this->module->drag_position = this->clampToDrawArea(this->module->drag_position.plus(e.mouseDelta.div(zoom)));
    }

    void onHover(const event::Hover &e) override
    {
        VoxglitchWidget::onHover(e);
        e.consume(this);

        if (this->module->tablet_mode)
        {
            this->module->drag_position = this->clampToDrawArea(e.pos);
        }
    }

    void step() override
    {
        VoxglitchWidget::step();
    }
};
