class DigitalSequencerTooltip : public VoxglitchTooltip
{
private:
    SequencerDisplayConfig *config = nullptr;
    float offset_x = 30.0;
    float offset_y = -15.0;

public:

    DigitalSequencerTooltip(SequencerDisplayConfig *cfg)
        : VoxglitchTooltip(10.0, cfg->draw_area_height - 20.0),  // Calling base class constructor
          config(cfg)                                  // Initializing member variable
    {
    }

    void setAttributes(std::string label, int column, float mouse_y)
    {
        setLabel(label);

        if (config)
        {
            setX(offset_x + (column * (config->bar_width + config->bar_horizontal_padding)));
            setY(offset_y + mouse_y);
        }
    }

    void draw(const DrawArgs &args) override
    {
        // Only draw the tooltip if config is not nullptr, and other draw conditions from the base class are met
        if (config) 
        {
            VoxglitchTooltip::draw(args); 
        }
    }
};
