#pragma once

class VoxglitchTooltip : public Widget
{
protected:
    bool draw_tooltip = false;
    unsigned int tooltip_timer = 0;
    std::string label = "Tooltip";
    float x = 0.0;
    float y = 0.0;
    float max_y = 0.0;
    float min_y = 0.0;
    float tooltip_height = 20;
    float text_horizontal_padding = 10.0;

public:

    VoxglitchTooltip(float min_y, float max_y)
    {
        this->min_y = min_y;
        this->max_y = max_y;
    }

    void drawTooltip(const DrawArgs &args)
    {
        const auto vg = args.vg;

        // Ensure that the tooltip should be drawn
        if (!draw_tooltip) {
            return;
        }

        nvgSave(vg);

        // This must happen before the bounds are computed
        nvgFontSize(vg, 13);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgTextLetterSpacing(vg, -1);

        // Calculate tooltip dimensions based on text content
        float bounds[4];
        float text_width = nvgTextBounds(vg, 0, 0, label.c_str(), NULL, bounds);
        float tooltip_width = text_width + text_horizontal_padding;

        // Compute hight of text
        // float text_height = bounds[3] - bounds[1];

        // Draw tooltip background
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, tooltip_width, tooltip_height, 2);
        nvgFillColor(vg, nvgRGBA(20, 20, 20, 250));
        nvgFill(vg);

        // Draw tooltip background outline
        nvgStrokeColor(vg, nvgRGBA(255, 215, 21, 100));  // A subtle light border; RGBA white with low opacity
        nvgStrokeWidth(vg, 1.0f);  // Change the width if you want a thicker or thinner border
        nvgStroke(vg);  // This actually draws the border

        // Set font attributes _again_ after drawing the background
        nvgFontSize(vg, 13);
        nvgFillColor(vg, nvgRGBA(255, 215, 20, 0xff));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgTextLetterSpacing(vg, -1);

        // Draw tooltip text
        nvgText(vg, x + (tooltip_width / 2), y + (tooltip_height / 2), label.c_str(), NULL);

        nvgRestore(vg);

        // Reduce the timer on every step, and disable drawing if it reaches zero
        if (tooltip_timer > 0) 
        {
            tooltip_timer--;
            draw_tooltip = (tooltip_timer > 0);
        }
    }

    void setDrawTooltip(bool draw)
    {
        draw_tooltip = draw;
    }

    void resetTimer(unsigned int new_time)
    {
        tooltip_timer = new_time;
    }

    void activateTooltip(unsigned int timer_value = 2)
    {
        setDrawTooltip(true);
        resetTimer(timer_value);
    }
    
    void deactivateTooltip()
    {
        setDrawTooltip(false);
        resetTimer(0);
    }

    void setLabel(std::string label)
    {
        this->label = label;
    }

    void setX(float x)
    {
        this->x = x;
    }

    void setY(float y)
    {
        this->y = clamp(y, min_y, max_y);
    }

};
