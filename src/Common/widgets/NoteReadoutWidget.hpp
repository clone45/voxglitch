struct NoteReadoutWidget : TransparentWidget
{
    std::string *display_string_ptr = NULL;
    std::string default_string = "0";

    float box_size_x = 45.0;
    float box_size_y = 30.0;

    float box_pos_x = 0;
    float box_pos_y = 0;

    NoteReadoutWidget(std::string default_string)
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
        
        std::string text_to_display = display_string_ptr != NULL ? *display_string_ptr : "";

        // Draw black background
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 3.0);
        nvgFillColor(vg, nvgRGB(0, 0, 0));
        nvgFill(vg);

        std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
        if (font)
        {
            // Set common font attributes
            nvgFontSize(args.vg, 18);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextLetterSpacing(args.vg, 0);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

            // Check if there's a sharp symbol and split the string
            size_t sharpPos = text_to_display.find('#');
            std::string note = sharpPos != std::string::npos ? text_to_display.substr(0, sharpPos) : text_to_display;
            std::string sharp = sharpPos != std::string::npos ? "#" : "";
            std::string octave = sharpPos != std::string::npos ? text_to_display.substr(sharpPos + 1) : "";

            // Calculate total width and starting x position for centered text
            float noteWidth = nvgTextBounds(vg, 0, 0, note.c_str(), NULL, NULL);
            float sharpWidth = nvgTextBounds(vg, 0, 0, sharp.c_str(), NULL, NULL);
            float octaveWidth = nvgTextBounds(vg, 0, 0, octave.c_str(), NULL, NULL);
            float totalWidth = noteWidth + sharpWidth + octaveWidth;
            float x = ((box.size.x - totalWidth) / 2.0);
            float y = (box.size.y / 2.0) + 1.5;

            // Draw the note
            nvgFillColor(args.vg, nvgRGBA(255, 215, 20, 0xff));
            nvgText(vg, x, y, note.c_str(), NULL);

            // Draw the sharp symbol higher up
            if (!sharp.empty())
            {
                nvgFontSize(args.vg, 14); // smaller font size for sharp
                nvgText(vg, x + noteWidth, y - 5, sharp.c_str(), NULL);
                nvgFontSize(args.vg, 18); // reset font size for octave
            }

            // Draw the octave
            nvgText(vg, x + noteWidth + sharpWidth, y, octave.c_str(), NULL);
        }

        nvgRestore(vg);
    }
};