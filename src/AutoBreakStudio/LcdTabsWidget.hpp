struct LcdTabsWidget : TransparentWidget
{
    AutobreakStudio *module;

    NVGcolor dark_background_color = nvgRGBA(31, 39, 41, 255);
    NVGcolor label_color = nvgRGBA(255, 255, 255, 255);

    LcdTabsWidget()
    {
        box.size = Vec(DRAW_AREA_WIDTH, LCD_TABS_HEIGHT);
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            const auto vg = args.vg;

            // Save the drawing context to restore later
            nvgSave(vg);

            if (module)
            {
                //
                // Display the tabs
                //
                for (unsigned int i = 0; i < 4; i++)
                {
                    drawTab(vg, i, "foo");
                }
            }

            nvgRestore(vg);
        }
    }

    void drawTab(NVGcontext *vg, unsigned int index, std::string label)
    {
        float x_position = (index * LCD_TABS_WIDTH) + (index * TABS_HORIZONTAL_PADDING);

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 0.0, LCD_TABS_WIDTH, LCD_TABS_HEIGHT);
        nvgFillColor(vg, dark_background_color);
        nvgFill(vg);

        nvgFontSize(vg, 14);
        nvgTextLetterSpacing(vg, 0);
        nvgFillColor(vg, label_color);
        nvgTextAlign(vg, NVG_ALIGN_LEFT);

        float y_position = 0;
        nvgTextBox(vg, y_position, x_position + 2.0, 900.0, label.c_str(), NULL);
    }
};