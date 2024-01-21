#pragma once

struct SequencerView : VoxglitchWidget
{
    Vec drag_position;
    float width = 0;
    float height = 0;
    float bar_width = 0;
    float bar_horizontal_padding = 0;
    unsigned int max_sequencer_steps = 0;

    NVGcolor bright_background_color = nvgRGBA(37, 56, 59, 255);
    NVGcolor dark_background_color = nvgRGBA(31, 39, 41, 255);
    NVGcolor current_step_highlight_color = nvgRGBA(255, 255, 255, 250);
    NVGcolor lesser_step_highlight_color = nvgRGBA(255, 255, 255, 150);
    NVGcolor default_step_highlight_color = nvgRGBA(255, 255, 255, 10);
    NVGcolor sequence_position_highlight_color = nvgRGBA(255, 255, 255, 20);
    NVGcolor overlay_color = nvgRGBA(0, 100, 116, 28);

    // Constructor
    SequencerView(float width, float height, float bar_width, float bar_horizontal_padding, unsigned int max_sequencer_steps)
    {
        this->width = width;
        this->height = height;
        this->bar_width = bar_width;
        this->bar_horizontal_padding = bar_horizontal_padding;
        this->max_sequencer_steps = max_sequencer_steps;

        box.size = Vec(width, height);
    }

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

    void drawVerticalGuildes(NVGcontext *vg, double height)
    {
        for (unsigned int i = 1; i < 8; i++)
        {
            nvgBeginPath(vg);
            int x = (i * 4 * bar_width) + (i * 4 * bar_horizontal_padding);
            nvgRect(vg, x, 0, 1, height);
            nvgFillColor(vg, nvgRGBA(240, 240, 255, 40));
            nvgFill(vg);
        }
    }

    void drawOverlay(NVGcontext *vg, double width, double height)
    {
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, width, height);
        nvgFillColor(vg, overlay_color);
        nvgFill(vg);
    }

    void drawBar(NVGcontext *vg, double position, double height, double container_height, NVGcolor color)
    {
        nvgBeginPath(vg);
        nvgRect(vg, (position * bar_width) + (position * bar_horizontal_padding), container_height - height, bar_width, height);
        nvgFillColor(vg, color);
        nvgFill(vg);
    }
};
