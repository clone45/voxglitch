struct DrumRandomizerReadoutWidget : TransparentWidget
{
    DrumRandomizer *module;

    float *value_pointer = NULL;
    bool show_decimal_points = false;

    DrumRandomizerReadoutWidget(bool show_decimal_points = false)
    {
        this->show_decimal_points = show_decimal_points;
        box.size = Vec(56.7845, 15.0384);
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

        std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
        if (font)
        {
            nvgFontSize(args.vg, 10);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextLetterSpacing(args.vg, 0);
            nvgFillColor(args.vg, nvgRGBA(245, 236, 229, 0xff));
        }

        // nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgTextAlign(vg, NVG_ALIGN_CENTER);

        // void nvgTextBox(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end);
        nvgTextBox(args.vg, 0, 11, 56.7845, text_to_display.c_str(), NULL);

        nvgRestore(vg);
    }
};