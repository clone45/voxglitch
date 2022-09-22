struct LcdTabsWidget : TransparentWidget
{
    AutobreakStudio *module;

    unsigned int selected_tab = 0;

    SequencerDisplayABS *sequencer_displays[NUMBER_OF_SEQUENCERS];

    NVGcolor tab_color_default = nvgRGBA(48, 75, 79, 255);
    NVGcolor tab_color_selected = nvgRGBA(68, 95, 99, 255);
    NVGcolor label_color_deselected = nvgRGBA(255, 255, 255, 100);
    NVGcolor label_color_selected = nvgRGBA(255, 255, 255, 200);

    std::string tab_labels[NUMBER_OF_TABS] = {
        "Position",
        "Sample",
        "Volume",
        "Pan",
        "Reverse",
        "Ratchet"
    };

    LcdTabsWidget(SequencerDisplayABS *sequencer_displays[4])
    {
        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            this->sequencer_displays[i] = sequencer_displays[i];
        }
    }

    LcdTabsWidget(bool draw_in_library)
    {
        // do nothing
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
                for (unsigned int i = 0; i < NUMBER_OF_TABS; i++)
                {
                    drawTab(vg, i, tab_labels[i]);
                }
            }
            else
            {
                // I know that this is redundant, but perhaps in the future, 
                // bringing attention to the fact that module may be undefined
                // might avoid bugs.
                for (unsigned int i = 0; i < NUMBER_OF_TABS; i++)
                {
                    drawTab(vg, i, tab_labels[i]);
                }     
            }

            nvgRestore(vg);
        }
    }

    void drawTab(NVGcontext *vg, unsigned int index, std::string label)
    {
        float x_position = (index * LCD_TABS_WIDTH) + (index * TABS_HORIZONTAL_PADDING);
        NVGcolor tab_color;
        NVGcolor label_color;

        if (index == selected_tab)
        {
            tab_color = tab_color_selected;
            label_color = label_color_selected;
        }
        else
        {
            tab_color = tab_color_default;
            label_color = label_color_deselected;
        }

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 0.0, LCD_TABS_WIDTH, LCD_TABS_HEIGHT);
        nvgFillColor(vg, tab_color);
        nvgFill(vg);

        nvgFontSize(vg, 12);
        nvgTextLetterSpacing(vg, 0);
        nvgFillColor(vg, label_color);
        nvgTextAlign(vg, NVG_ALIGN_CENTER);

        float y_position = 0;
        nvgTextBox(vg, x_position, y_position + 14.0, LCD_TABS_WIDTH, label.c_str(), NULL);
    }

    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            e.consume(this);

            unsigned int new_tab = e.pos.x / (LCD_TABS_WIDTH + TABS_HORIZONTAL_PADDING);
            new_tab = clamp(new_tab, 0, 5);

            this->switchTabs(new_tab);
        }
    }

    void switchTabs(unsigned int tab)
    {
        selected_tab = tab;

        for (unsigned int i = 0; i < NUMBER_OF_SEQUENCERS; i++)
        {
            sequencer_displays[i]->hide();
            if (tab == i)
                sequencer_displays[i]->show();
        }
    }
};