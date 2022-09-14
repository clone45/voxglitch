struct LcdTabsWidget : TransparentWidget
{
    AutobreakStudio *module;

    unsigned int selected_tab = 0;

    SequencerDisplayABS *sequencer_displays[NUMBER_OF_SEQUENCERS];

    NVGcolor tab_color_default = nvgRGBA(48, 75, 79, 255);
    NVGcolor tab_color_selected = nvgRGBA(68, 95, 99, 255);
    NVGcolor label_color = nvgRGBA(255, 255, 255, 255);

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

        box.size = Vec(DRAW_AREA_WIDTH, LCD_TABS_HEIGHT);
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

            nvgRestore(vg);
        }
    }

    void drawTab(NVGcontext *vg, unsigned int index, std::string label)
    {
        float x_position = (index * LCD_TABS_WIDTH) + (index * TABS_HORIZONTAL_PADDING);
        NVGcolor tab_color;

        if (index == selected_tab)
        {
            tab_color = tab_color_selected;
        }
        else
        {
            tab_color = tab_color_default;
        }

        nvgBeginPath(vg);
        nvgRect(vg, x_position, 0.0, LCD_TABS_WIDTH, LCD_TABS_HEIGHT);
        nvgFillColor(vg, tab_color);
        nvgFill(vg);

        nvgFontSize(vg, 14);
        nvgTextLetterSpacing(vg, 0);
        nvgFillColor(vg, label_color);
        nvgTextAlign(vg, NVG_ALIGN_LEFT);

        float y_position = 0;
        nvgTextBox(vg, x_position, y_position + 12.0, 900.0, label.c_str(), NULL);
    }

    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            e.consume(this);
            this->switchTabs(e.pos.x / (LCD_TABS_WIDTH + TABS_HORIZONTAL_PADDING));
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