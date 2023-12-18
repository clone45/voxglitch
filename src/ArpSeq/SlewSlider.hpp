#include "../Common/components/VoxglitchTooltip.hpp"
#include "../Common/components/DigitalHSliderTooltip.hpp"

struct SlewSlider : DigitalHorizontalSlider
{
    float WIDTH = 74.0f;
    float HEIGHT = 10.0f;

    DigitalHSliderTooltip *tooltip = new DigitalHSliderTooltip();
    std::function<std::string()> tooltipCallback;

    SlewSlider(float *value)
    : DigitalHorizontalSlider(value)  // Call the constructor of the base class
    {
        // You can now initialize SlewSlider's additional members, if any
    }

    //void draw(const DrawArgs &args) override

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            DigitalHorizontalSlider::draw(args);

            if(tooltip && tooltipCallback)
            {
                std::string label = tooltipCallback();
                tooltip->setAttributes(label, drag_position.x, 0.0);
                tooltip->drawTooltip(args);
            }
        }
    }

    void onButton(const event::Button &e) override
    {
        DigitalHorizontalSlider::onButton(e);
    }

    void onDragMove(const event::DragMove &e) override
    {
        DigitalHorizontalSlider::onDragMove(e);
        tooltip->activateTooltip();
    }

    void onDragEnd(const event::DragEnd &e) override
    {
        DigitalHorizontalSlider::onDragEnd(e);
        tooltip->deactivateTooltip();
    }
};
