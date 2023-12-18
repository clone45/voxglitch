class DigitalHSliderTooltip : public VoxglitchTooltip
{
private:
    float offset_x = -15.0;
    float offset_y = -15.0;

public:

    DigitalHSliderTooltip()
        : VoxglitchTooltip(10.0, -20.0)
    {
    }

    void setAttributes(std::string label, float mouse_x, float mouse_y)
    {
        setLabel(label);

        setX(offset_x + mouse_x);
        setY(offset_y + mouse_y);
    }

    void drawTooltip(const DrawArgs &args)
    {
        VoxglitchTooltip::drawTooltip(args); 
    }
};
