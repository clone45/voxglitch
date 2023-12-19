struct NoteReadoutWidget : TransparentWidget
{
    std::string *display_string_ptr = NULL;
    std::string default_string = "0";

    float box_size_x = 45.0;
    float box_size_y = 30.0;
    int font_size = 18;

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
            nvgFontSize(args.vg, font_size);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextLetterSpacing(args.vg, 0);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

            // Check for sharp or flat symbol
            size_t symbolPos = text_to_display.find_first_of("#b");
            std::string note = symbolPos != std::string::npos ? text_to_display.substr(0, symbolPos) : text_to_display;
            std::string symbol = symbolPos != std::string::npos ? text_to_display.substr(symbolPos, 1) : "";
            std::string octave = symbolPos != std::string::npos ? text_to_display.substr(symbolPos + 1) : "";

            // Calculate total width and starting x position for centered text
            float noteWidth = nvgTextBounds(vg, 0, 0, note.c_str(), NULL, NULL);
            float symbolWidth = nvgTextBounds(vg, 0, 0, symbol.c_str(), NULL, NULL);
            float octaveWidth = nvgTextBounds(vg, 0, 0, octave.c_str(), NULL, NULL);
            float totalWidth = noteWidth + symbolWidth + octaveWidth;
            float x = ((box.size.x - totalWidth) / 2.0);
            float y = (box.size.y / 2.0) + 1.5;

            // Draw the note
            nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 0xff));
            nvgText(vg, x, y, note.c_str(), NULL);

            // Draw the sharp or flat symbol
            if (!symbol.empty())
            {
                nvgFontSize(args.vg, font_size - (symbol == "#" ? 4 : 2)); // adjust font size based on symbol
                nvgText(vg, x + noteWidth, y - (symbol == "#" ? 5 : 2), symbol.c_str(), NULL);
                nvgFontSize(args.vg, font_size); // reset font size for octave
            }

            // Draw the octave
            nvgText(vg, x + noteWidth + symbolWidth, y, octave.c_str(), NULL);
        }

        nvgRestore(vg);
    }

};