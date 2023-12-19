struct ArpSequencerDisplay : TransparentWidget
{
    ArpSeq *module;
    SequencerDisplayConfig *config = NULL;

    Vec drag_position;

    NVGcolor bar_background_color = nvgRGBA(94, 78, 7, 255);
    NVGcolor bar_default_color = nvgRGBA(168, 142, 13, 255);
    NVGcolor bar_current_color = nvgRGBA(255, 215, 20, 255);
    NVGcolor bar_out_of_range_overlay_color = nvgRGBA(0, 0, 0, 60);

    void onDragStart(const event::DragStart &e) override
    {
        TransparentWidget::onDragStart(e);
    }

    void onDragEnd(const event::DragEnd &e) override
    {
        TransparentWidget::onDragEnd(e);
    }

    // Allow for changing between sequences using the number keys

    void step() override
    {
        TransparentWidget::step();
    }

    void onEnter(const event::Enter &e) override
    {
        TransparentWidget::onEnter(e);
    }

    void onLeave(const event::Leave &e) override
    {
        TransparentWidget::onLeave(e);
    }

    bool keypressRight(const event::HoverKey &e)
    {
        if (e.key == GLFW_KEY_RIGHT)
            e.consume(this);
        if (e.key == GLFW_KEY_RIGHT && e.action == GLFW_PRESS)
            return true;
        return false;
    }

    bool keypressLeft(const event::HoverKey &e)
    {
        if (e.key == GLFW_KEY_LEFT)
            e.consume(this);
        if (e.key == GLFW_KEY_LEFT && e.action == GLFW_PRESS)
            return true;
        return false;
    }

    bool keypressUp(const event::HoverKey &e)
    {
        if (e.key == GLFW_KEY_UP)
            e.consume(this);
        if (e.key == GLFW_KEY_UP && e.action == GLFW_PRESS)
            return true;
        return false;
    }

    bool keypressDown(const event::HoverKey &e)
    {
        if (e.key == GLFW_KEY_DOWN)
            e.consume(this);
        if (e.key == GLFW_KEY_DOWN && e.action == GLFW_PRESS)
            return true;
        return false;
    }

    void drawVerticalGuildes(NVGcontext *vg, double height, unsigned int number_of_guides = 4)
    {
        for (unsigned int i = 1; i < number_of_guides; i++)
        {
            nvgBeginPath(vg);
            int x = (i * 4 * config->bar_width) + (i * 4 * config->bar_horizontal_padding);
            nvgRect(vg, x, 0, 1, height);
            nvgFillColor(vg, nvgRGBA(240, 240, 255, 40));
            nvgFill(vg);
        }
    }

    void drawOverlay(NVGcontext *vg, double width, double height)
    {
        /*
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, width, height);
        nvgFillColor(vg, overlay_color);
        nvgFill(vg);
        */
    }
};
