//
//==============================================================================
// LCDRatchetDisplay
//==============================================================================
//
struct LCDRatchetDisplay : LCDDisplay
{
    float column_width = 0;
    float column_height = 0;
    float column_padding = 8.0;
    float row_vertical_padding = 6.0;
    float cell_width = 0;
    float cell_height = 0;
    float number_of_columns = 2;
    float cell_padding = 2.0;

    // float lcd_padding_top = (lcd_height - ((8 * cell_height) + (padding * 7))) / 2.0;
    // float lcd_padding_left = lcd_padding_top;

    LCDRatchetDisplay(GrooveBox *module)
    {
        this->module = module;

        box.pos.x = this->box_pos_x;
        box.pos.y = this->box_pos_y;
        box.size = Vec(this->box_width, this->box_height);

        column_width = (box.size.x - (display_padding * 2.0) - column_padding) / number_of_columns;
        column_height = box.size.y - (display_padding * 2.0);

        cell_width = (column_width - (cell_padding * 6.0)) / 7.0;
        cell_height = (column_height - (row_vertical_padding * 7.0)) / 8.0;
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            const auto vg = args.vg;

            if (module && module->lcd_screen_mode == module->RATCHET)
            {
                nvgSave(vg);

                // Determine the selected ratchet pattern
                float ratchet_float_value = module->selected_track->sample_playback_settings[module->visualizer_step].ratchet;
                unsigned int ratchet_pattern_index = ratchet_float_value * NUMBER_OF_RATCHET_PATTERNS;

                float column_x = 0;

                for (unsigned int i = 0; i < NUMBER_OF_RATCHET_PATTERNS; i++)
                {
                    // unsigned int column = i / (NUMBER_OF_RATCHET_PATTERNS / 2);
                    
                    if(i < NUMBER_OF_RATCHET_PATTERNS / 2)
                    {
                        column_x = display_padding;  // (column * (box.size.x / 2.0)) - (padding * column);
                    }
                    else
                    {
                        column_x = display_padding + column_width + column_padding;
                    }
                    
                    float y = display_padding + ((i%(NUMBER_OF_RATCHET_PATTERNS/2)) * (cell_height + row_vertical_padding));  // ((cell_height + padding) * (i % (NUMBER_OF_RATCHET_PATTERNS / 2)));

                    for (unsigned int ratchet_array_index = 0; ratchet_array_index < 7; ratchet_array_index++)
                    {
                        float x = column_x + (ratchet_array_index * (cell_width + cell_padding)); // ((ratchet_array_index * cell_width) + x + (padding * ratchet_array_index));

                        nvgBeginPath(vg);
                        nvgRect(vg, x, y, cell_width, cell_height);

                        bool ratchet_array_boolean = ratchet_patterns[i][ratchet_array_index];

                        if (ratchet_array_boolean)
                        {
                            nvgFillColor(vg, module->lcd_color_scheme.getLightColor()); // Friendly blue highlight
                        }
                        else
                        {
                            nvgFillColor(vg, module->lcd_color_scheme.getDarkColor()); // Grey color
                        }
                        nvgFill(vg);
                    }

                    // highlight row
                    if(ratchet_pattern_index == i)
                    {
                        nvgBeginPath(vg);
                        nvgRect(vg, column_x, y, column_width, cell_height);
                        nvgFillColor(vg, module->lcd_color_scheme.getHighlightOverlay());
                        nvgFill(vg);
                    }
                }

                nvgRestore(vg);
            }
        }
        Widget::drawLayer(args, layer);
    }
};