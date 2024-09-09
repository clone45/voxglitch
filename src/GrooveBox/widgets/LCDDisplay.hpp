//
// LCDDisplay
//
struct LCDDisplay : TransparentWidget
{
    GrooveBox *module;

    float box_pos_x = 249.011;
    float box_pos_y = 77.494;
    float box_width = 389.909;
    float box_height = 140.764;

    float display_padding = 8.0;

    LCDDisplay()
    {
        box.size.x = box_width;
        box.size.y = box_height;
        box.pos.x = box_pos_x;
        box.pos.y = box_pos_y;
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            const auto vg = args.vg;

            nvgBeginPath(vg);
            nvgRect(vg, 0, 0, box.size.x, box.size.y);
            nvgFillColor(vg, LCDColorScheme::getBackgroundColor());
            nvgFill(vg);
        }
        Widget::drawLayer(args, layer);
    }
};
