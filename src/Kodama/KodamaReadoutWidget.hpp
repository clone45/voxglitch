struct KodamaReadoutWidget : TransparentWidget
{
    Kodama *module;

    KodamaReadoutWidget()
    {
        // box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
        box.size = Vec(56.7845, 15.0384);
    }

    void draw(const DrawArgs &args) override
    {
        const auto vg = args.vg;

        // Save the drawing context to restore later
        nvgSave(vg);

        // Debugging code for draw area
        /*
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, nvgRGBA(130, 10, 120, 100));
        nvgFill(vg);
        */

        std::string text_to_display = "0";

        if (module)
        {
            text_to_display = std::to_string(module->selected_sequence);
        }

        std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/ShareTechMono-Regular.ttf"));
        if (font)
        {
            nvgFontSize(args.vg, 10);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextLetterSpacing(args.vg, 0);
            nvgFillColor(args.vg, nvgRGBA(245, 236, 229, 0xff));
        }

        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        // void nvgTextBox(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end);
        nvgTextBox(args.vg, 4, 8, 700, text_to_display.c_str(), NULL);

        nvgRestore(vg);
    }
};