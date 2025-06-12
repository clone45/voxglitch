struct OneZeroReadoutWidget : TransparentWidget
{
    OneZero *module;

    OneZeroReadoutWidget()
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
        nvgFillColor(vg, nvgRGBA(230, 10, 120, 100));
        nvgFill(vg);
        */

        std::string text_to_display = "0";

        if (module)
        {
            if(module->sequences.empty())
            {
                text_to_display = "NO DATA";
            }
            else
            {
                text_to_display = std::to_string(module->real_selected_sequence);
            }
        }

        std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
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

    void onDoubleClick(const event::DoubleClick &e) override
    {
#ifdef USING_CARDINAL_NOT_RACK
        OneZero *module = this->module;
        async_dialog_filebrowser(false, NULL, NULL, "Open txt", [module](char* path) {
            if (path != NULL) {
                module->loadData(path);
                module->path = path;
            }
            free(path);
        });
#else
      std::string path = module->selectFileVCV();
      module->loadData(path);
      module->path = path;
#endif
    }
};
