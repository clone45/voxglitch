struct DigitalSelect : Widget
{
    std::vector<std::string> items = {
        "Chromatic",
        "Major",
        "Minor",
        "Pentatonic",
        "Dorian",
        "Phrygian",
        "Lydian",
        "Mixolydian",
        "Harmonic Minor",
        "Melodic Minor",
        "Blues",
        "Whole Tone",
        "Diminished"
    };

    bool is_open = false;
    unsigned int *selected_index = 0;
    float item_height = 20.0f; // Height for each item in the list
    float slight_overlap = 0.3f;
    NVGcolor label_color = nvgRGB(234, 202, 63);
    NVGcolor background_color = nvgRGB(30, 24, 2);
    NVGcolor highlight_color = nvgRGB(94, 78, 7);
    int hovered_index = -1; // -1 indicates no item is being hovered over

    DigitalSelect(unsigned int *selected_index)
    {
        this->selected_index = selected_index;
        box.size.x = 120.0f;
        box.size.y = item_height; // default height for one item
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            // Draw the selected item text
            setFontAttributes(args.vg);
            nvgText(args.vg, 0, item_height * 0.5, items[*selected_index].c_str(), NULL);

            // If open, draw the list
            if (is_open)
            {
                for (unsigned int i = 0; i < (unsigned int) items.size(); i++)
                {
                    nvgBeginPath(args.vg);
                    nvgRect(args.vg, 0, item_height * (i + 1), box.size.x, item_height + slight_overlap);
                    nvgFillColor(args.vg, background_color); // slightly darker background for list
                    nvgFill(args.vg);


                    // If this item is the hovered item, draw the highlight
                    if (i == (unsigned int)hovered_index)
                    {
                        nvgBeginPath(args.vg);
                        nvgRect(args.vg, 0, item_height * (i + 1), box.size.x, item_height);
                        nvgFillColor(args.vg, highlight_color); // Semi-transparent black
                        nvgFill(args.vg);
                    }

                    setFontAttributes(args.vg);

                    std::string itemText = items[i];
                    if(i == *selected_index) {
                        itemText += " \u2713"; // Appending the checkmark if it's the selected item
                    }
                    nvgText(args.vg, 8, item_height * (i + 1.5), itemText.c_str(), NULL);

                }
                box.size.y = item_height * (items.size() + 1);
            }
            else
            {
                box.size.y = item_height; // reset height
            }
        }
    }

    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            if (!is_open)
            {
                is_open = true;
            }
            else
            {
                // If the user clicks on the actively displayed label, then just close the list
                if(e.pos.y < item_height)
                {
                    is_open = false;
                    return;
                }
 
                int clicked_index = e.pos.y / item_height - 1;

                if (clicked_index < 0)
                {
                    is_open = false;
                }
                else if (clicked_index < (int) items.size())
                {
                    *selected_index = clicked_index;
                    is_open = false;
                }
            }
            e.consume(this);
        }
    }

    void onHover(const event::Hover &e) override
    {
        if (is_open)
        {
            hovered_index = e.pos.y / item_height - 1;
            if (hovered_index < 0 || hovered_index >= (int) items.size())
                hovered_index = -1; // Ensure valid range or set to -1 if out of bounds
        }
        else
        {
            hovered_index = -1;
        }

        Widget::onHover(e);
        e.consume(this);
    }

    void onLeave(const event::Leave &e) override
    {
        is_open = false;
        hovered_index = -1;
        Widget::onLeave(e);
    }

    void onHoverKey(const event::HoverKey &e) override
    {
        // If the escape key is pressed, then close the menu
        if (e.key == GLFW_KEY_ESCAPE && e.action == GLFW_PRESS)
        {
            is_open = false;
            e.consume(this);
        }
    }


    void setFontAttributes(NVGcontext* vg) 
    {
        nvgFontSize(vg, 12);
        nvgFontFaceId(vg, 0);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, label_color);
    }

    float getTextVerticalPosition(NVGcontext* vg, float itemHeight)
    {
        float ascent, descent, lineH;
        nvgTextMetrics(vg, &ascent, &descent, &lineH);

        // Calculate the vertical position to center the text
        return itemHeight * 0.5f + ascent - lineH * 0.5f;
    }

};
