struct BytebeatSegmentReadoutWidget : TransparentWidget
{
    std::string *display_string_ptr = NULL;
    std::string default_string = "0";

    float box_size_x = 45.0;
    float box_size_y = 30.0;

    float box_pos_x = 0;
    float box_pos_y = 0;

    BytebeatSegmentReadoutWidget(std::string default_string)
    {
        this->default_string = default_string;
        this->display_string_ptr = &this->default_string;

        box.size = Vec(box_size_x, box_size_y);
    }

    void draw(const DrawArgs &args) override
    {
        const auto vg = args.vg;

        // Save the drawing context to restore later
        nvgSave(vg);
        
        std::string text_to_display = "0";

        if (display_string_ptr != NULL)
        {
            text_to_display = *display_string_ptr;
        }
        else
        {
            text_to_display= "";
        }

        // Draw black background
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 3.0);
        nvgFillColor(vg, nvgRGB(0, 0, 0));
        nvgFill(vg);

        std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Segment14.ttf"));
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