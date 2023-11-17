struct DrumRandomizerReadoutWidget : TransparentWidget
{
    float *value_pointer = NULL;
    bool show_decimal_points = false;

    float box_size_x = 45.0;
    float box_size_y = 30.0;

    float box_pos_x = 0;
    float box_pos_y = 0;

    DrumRandomizerReadoutWidget(bool show_decimal_points = false)
    {
        this->show_decimal_points = show_decimal_points;
        box.size = Vec(box_size_x, box_size_y);
    }

    void draw(const DrawArgs &args) override
    {
        const auto vg = args.vg;

        // Save the drawing context to restore later
        nvgSave(vg);

        std::string text_to_display = "0";

        if (value_pointer != NULL)
        {
            if(show_decimal_points)
            {
                text_to_display = std::to_string(*value_pointer);
                text_to_display = text_to_display.substr(0,4);
            }
            else
            {
                text_to_display = std::to_string((int) *value_pointer);
            }
        }
        else
        {
            text_to_display= "0";
        }

        // Draw black background
        nvgBeginPath(vg);
        // nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 3.0);
        nvgFillColor(vg, nvgRGB(0, 0, 0));
        nvgFill(vg);

        std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Segment14.ttf"));
        if (font)
        {
            // Set common font attributes
            nvgFontSize(args.vg, 14);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextLetterSpacing(args.vg, 0);
            nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);

            // Draw a faded version of the 14 segment display
            nvgFillColor(args.vg, nvgRGBA(30, 30, 30, 0xff));
            nvgTextBox(args.vg, -16.0, box_size_y / 2.0, 56.7845, "~~~", NULL);

            // Set the color for the 14 segment text
            nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));

            nvgTextBox(args.vg, -16.0, box_size_y / 2.0, 56.7845, text_to_display.c_str(), NULL);
        }

        nvgRestore(vg);
    }
};