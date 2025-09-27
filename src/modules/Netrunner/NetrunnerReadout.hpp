struct NetrunnerReadout : TransparentWidget
{
    Netrunner *module;
    std::shared_ptr<Font> font;
    std::string text_to_display = "";
    bool mouse_lock = false;
    unsigned int window_start = 0;
    unsigned int window_end = 0;
    unsigned int hover_row = 0;
    bool show_hover_effect = false;

    void draw(const DrawArgs &args) override
    {

        std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
        if (font)
        {
            nvgFontSize(args.vg, 11);
            nvgFontFaceId(args.vg, font->handle);
            nvgTextLetterSpacing(args.vg, 0);
        }

        if (module)
        {
            text_to_display = "";

            unsigned int number_of_sample = module->sample_players.size();

            if (number_of_sample > 0)
            {
                window_start = 0;
                window_end = number_of_sample;

                if (!(number_of_sample < NUMBER_OF_SAMPLE_DISPLAY_ROWS))
                {
                    unsigned int half_display_location = (NUMBER_OF_SAMPLE_DISPLAY_ROWS / 2);
                    unsigned int number_of_loaded_samples = number_of_sample;

                    if (module->selected_sample_slot > half_display_location)
                    {
                        window_start = module->selected_sample_slot - half_display_location;
                        window_end = module->selected_sample_slot + half_display_location + 1;

                        if (window_end > number_of_loaded_samples)
                        {
                            window_start = number_of_loaded_samples - NUMBER_OF_SAMPLE_DISPLAY_ROWS;
                            window_end = number_of_loaded_samples;
                        }

                        if (window_end > number_of_loaded_samples)
                            window_end = number_of_loaded_samples;
                    }
                    else
                    {
                        window_start = 0;
                        window_end = NUMBER_OF_SAMPLE_DISPLAY_ROWS;
                    }
                }

                for (unsigned int i = window_start; i < window_end; i++)
                {
                    text_to_display = module->sample_players[i].sample.display_name;
                    text_to_display.resize(22);

                    if (i == module->selected_sample_slot || (show_hover_effect && hover_row == i))
                    {
                        nvgFillColor(args.vg, nvgRGBA(150, 219, 234, 0xff));
                    }
                    else
                    {
                        nvgFillColor(args.vg, nvgRGBA(102, 151, 163, 0xff));
                    }

                    nvgText(args.vg, 0, 6.3 + ((i - window_start) * 14), text_to_display.c_str(), NULL);
                }
            }
        }
        else
        {
            std::string dummy_filenames[20] = {
                "sample_001.wav",
                "sample_002.wav",
                "sample_003.wav",
                "sample_004.wav",
                "sample_005.wav",
                "sample_006.wav",
                "sample_007.wav",
                "sample_008.wav",
                "sample_009.wav",
                "sample_010.wav",
                "sample_011.wav",
                "sample_012.wav",
                "sample_013.wav",
                "sample_014.wav",
                "sample_015.wav",
                "sample_016.wav",
                "sample_017.wav",
                "sample_018.wav",
                "sample_019.wav",
                "sample_020.wav"};

            for (unsigned int i = 0; i < NUMBER_OF_SAMPLE_DISPLAY_ROWS; i++)
            {
                text_to_display = dummy_filenames[i];
                text_to_display.resize(22);

                if (i == 4)
                {
                    nvgFillColor(args.vg, nvgRGBA(150, 219, 234, 0xff));
                }
                else
                {
                    nvgFillColor(args.vg, nvgRGBA(102, 151, 163, 0xff));
                }

                nvgText(args.vg, 0, 6.3 + ((i - window_start) * 14), text_to_display.c_str(), NULL);
            }
        }
    }
};