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

    /*
    void draw(const DrawArgs &args) override
    {
        const auto vg = args.vg;

        if (module)
        {
            nvgSave(vg);
            nvgBeginPath(vg);
            nvgFillColor(vg, module->lcd_color_scheme.getBackgroundColor());
            nvgRect(vg, 0, 0, 20, 20);
            nvgStroke(vg);
            nvgRestore(vg);

            // TransparentWidget::draw(args);
        }
    }
    */

};
