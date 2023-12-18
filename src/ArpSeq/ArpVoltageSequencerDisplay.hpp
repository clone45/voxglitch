#include "../Common/components/VoxglitchTooltip.hpp"
#include "../Common/components/DigitalSequencerTooltip.hpp"

struct ArpVoltageSequencerDisplay : ArpSequencerDisplay
{
    ArpSeq *module;
    VoltageSequencer *current_sequencer = NULL;
    DigitalSequencerTooltip *tooltip;
    std::function<std::string(int, float)> tooltipCallback;

    bool shift_key = false;
    bool ctrl_key = false;
    bool draw_horizontal_guide = false;
    bool double_click = false;

    int previous_shift_sequence_column = 0;
    int shift_sequence_column = 0;

    int previous_control_sequence_column = 0;
    int control_sequence_column = 0;

    ArpVoltageSequencerDisplay(VoltageSequencer *current_sequencer, SequencerDisplayConfig *cfg)
    {
        this->config = cfg;
        this->current_sequencer = current_sequencer;

        config->bar_width = (config->draw_area_width - ((config->max_sequencer_steps - 1) * config->bar_horizontal_padding)) / (float) config->max_sequencer_steps;

        // Set the tooltip
        tooltip = new DigitalSequencerTooltip(cfg);
        
        box.size = Vec(config->draw_area_width, config->draw_area_height);
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            const auto vg = args.vg;
            float value = 0.0;
            NVGcolor bar_color;

            // Save the drawing context to restore later
            nvgSave(vg);

            if (module)
            {
                // Get a pointer to the Voltage Sequencer
                VoltageSequencer *sequencer = current_sequencer;

                if (sequencer->polarity == VoltageSequencer::BIPOLAR) 
                {
                    value = (2.0 * value) - 1.0;
                    value = clamp(value, 0.0, 1.0);
                }

                //
                // Display the pattern
                //

                for (int column = 0; column < config->max_sequencer_steps; column++)
                {
                    value = sequencer->getValue(column);

                    // Draw the background bar
                    bar_color = bar_background_color;
                    drawColumnLayer(vg, column, bar_color);


                    if (column == sequencer->getPlaybackPosition())
                    {
                        bar_color = bar_current_color;
                    }
                    else
                    {
                        bar_color = bar_default_color;
                    }

                    // Now draw the rectangle for the bar
                    // if (value != 0 || sequencer->polarity == VoltageSequencer::BIPOLAR) 
                    //{
                        drawColumn(vg, column, value, bar_color);
                    // }
                }

                // Draw a horizontal 0-indicator by bi-polar sequencers
                if (sequencer->polarity == VoltageSequencer::BIPOLAR)
                {
                    // This calculation for y takes advance of the fact that all
                    // ranges that would have the 0-guide visible are symmetric, so
                    // it will need updating if non-symmetric ranges are added.
                    double y = config->draw_area_height / 2.0;

                    nvgBeginPath(vg);
                    nvgRect(vg, 0, y, config->draw_area_width, 1.0);
                    nvgFillColor(vg, nvgRGBA(0, 0, 0, 90));
                    nvgFill(vg);
                }

                // Draw the tooltip
                tooltip->drawTooltip(args);
            }
            else // Draw a demo sequence so that the sequencer looks nice in the library selector
            {
                double demo_sequence[16] = {0.0, 0.0, 0.25, 1.00, 0.50, 0.50, 0.25, 0.75, 0.0, 0.0, 0.25, 0.75, 0.0, 0.0, 0.25, .75};

                float draw_area_width = 363.03758;
                float draw_area_height = config->draw_area_height;
                float bar_horizontal_padding = 0.8;
                float bar_width = (draw_area_width / (double)MAX_SEQUENCER_STEPS) - bar_horizontal_padding;
                float width = bar_width;
                NVGcolor bar_color = bar_default_color;

                for (unsigned int i = 0; i < MAX_SEQUENCER_STEPS; i++)
                {
                    float value = demo_sequence[i];

                    float x = (i * bar_width) + (i * bar_horizontal_padding);
                    float height = value * draw_area_height;
                    float y = draw_area_height - height;

                    nvgBeginPath(vg);
                    nvgRect(vg, x, y, width, height);
                    nvgFillColor(vg, bar_color);
                    nvgFill(vg);
                }
            }

            // drawOverlay(vg, config->overlay_width, config->draw_area_height);

            nvgRestore(vg);
        }
    }

    void drawColumn(NVGcontext *vg, unsigned int column, float value, NVGcolor color)
    {
        double height = 0.0;
        double y = 0.0;
        double x = (column * config->bar_width) + (column * config->bar_horizontal_padding);
        double width = config->bar_width;

        // Bipolar mode
        if (current_sequencer && (current_sequencer->polarity == VoltageSequencer::BIPOLAR)) 
        {
            if (value > 0.5)
            {
                float half_height = config->draw_area_height / 2;
                height = (value - 0.5) * config->draw_area_height;
                y = half_height - height;
            }
            else
            {
                float half_height = config->draw_area_height / 2;
                y = half_height;
                height = half_height - ((2 * value) * half_height);
            }

            nvgBeginPath(vg);
            nvgRect(vg, x, y, width, height);
            nvgFillColor(vg, color);
            nvgFill(vg);
        }
        // Unipolar mode
        else if (value != 0)
        {
            height = value * config->draw_area_height;
            y = config->draw_area_height - height;

            nvgBeginPath(vg);
            nvgRect(vg, x, y, width, height);
            nvgFillColor(vg, color);
            nvgFill(vg);
        }


        // If the column is out of range of the sequencer window, then call
        // drawColumnLayer() to draw a semi-transparent layer over the column
        // to indicate that it is out of range.
        if (current_sequencer && (current_sequencer->isWithinWindow(column) == false))
        {
            drawColumnLayer(vg, column, bar_out_of_range_overlay_color);
        }

    }

    void drawColumnLayer(NVGcontext *vg, unsigned int column, NVGcolor color)
    {
        // Draw a rectangle that spans the entire height of the display
        // and is the width of the column
        nvgBeginPath(vg);
        nvgRect(vg, (column * config->bar_width) + (column * config->bar_horizontal_padding), 0, config->bar_width, config->draw_area_height);
        nvgFillColor(vg, color);
        nvgFill(vg);
    }


    //
    // void editBar(Vec mouse_position)
    //
    // Called when the user clicks to edit one of the sequencer values.  Sets
    // the sequencer value that the user has selected.
    //
    void editBar(Vec mouse_position)
    {
        float value = 0.0;

        if (module)
        {
            VoltageSequencer *sequencer = current_sequencer;

            float bar_width = (config->draw_area_width / (double)MAX_SEQUENCER_STEPS) - config->bar_horizontal_padding;
            int column = mouse_position.x / (bar_width + config->bar_horizontal_padding);
            column = clamp(column, 0, MAX_SEQUENCER_STEPS - 1);

            // I think that the y value should be the same for both polarities.  It's only
            // in the display that things are different.
            float clicked_y = config->draw_area_height - mouse_position.y;
            clicked_y = clamp(clicked_y, 0.0, config->draw_area_height);
            value = clicked_y / config->draw_area_height;

            sequencer->setValue(column, value);

            // Callback for the tooltip value and display
            tooltip->activateTooltip();
            std::string tooltipText = tooltipCallback(column, value);
            tooltip->setAttributes(tooltipText, column, mouse_position.y);
        }
    }


    void resetBar(Vec mouse_position)
    {
        if (module)
        {
            VoltageSequencer *sequencer = current_sequencer;

            float bar_width = (config->draw_area_width / (double)MAX_SEQUENCER_STEPS) - config->bar_horizontal_padding;
            int column = mouse_position.x / (bar_width + config->bar_horizontal_padding);
            column = clamp(column, 0, MAX_SEQUENCER_STEPS - 1);

            // float reset_value = (sequencer->polarity == VoltageSequencer::UNIPOLAR)  ? 0.0 : 0.5;
            float reset_value = sequencer->getDefault();

            sequencer->setValue(column, reset_value);
        }
    }

    void startShiftSequences(Vec mouse_position)
    {
        int clicked_column = mouse_position.x / (config->bar_width + config->bar_horizontal_padding);
        this->previous_shift_sequence_column = clicked_column;
        this->shift_sequence_column = clicked_column;
    }

    void dragShiftSequences(Vec mouse_position)
    {
        if (module)
        {
            VoltageSequencer *sequencer = current_sequencer;

            int drag_column = mouse_position.x / (config->bar_width + config->bar_horizontal_padding);
            int shift_offset = drag_column - this->shift_sequence_column;

            while (shift_offset < 0)
            {
                sequencer->shiftLeftInWindow();
                shift_offset++;
            }

            while (shift_offset > 0)
            {
                sequencer->shiftRightInWindow();
                shift_offset--;
            }

            this->shift_sequence_column = drag_column;
        }
    }

    void onButton(const event::Button &e) override
    {
        if (this->double_click == true)
        {
            this->double_click = false;
            resetBar(e.pos);
            return;
        }

        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            current_sequencer->history_manager.startSession();

            e.consume(this);
            drag_position = e.pos;

            if (this->shift_key == true)
            {
                startShiftSequences(drag_position);
            }
            else if (this->ctrl_key == true)
            {
                // startControlSequence(drag_position);
            }
            else
            {
                this->editBar(e.pos);
            }
        }

        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0)
        {
            e.consume(this);
            createContextMenu();
        }

        // If the left button was lifted, then remove the tooltip
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
        {
            tooltip->deactivateTooltip();

            current_sequencer->history_manager.endSession();
        }
    }

    void onDragMove(const event::DragMove &e) override
    {
        TransparentWidget::onDragMove(e);
        float zoom = getAbsoluteZoom();
        drag_position = drag_position.plus(e.mouseDelta.div(zoom));

        if (this->shift_key == true)
        {
            dragShiftSequences(drag_position);
        }
        else if (this->ctrl_key == true)
        {
            // dragControlSequence(drag_position);
        }
        else
        {
            if (e.button == GLFW_MOUSE_BUTTON_LEFT)
            {
                editBar(drag_position);
            }
        }
    }

    void onDragEnd(const event::DragEnd &e) override
    {
        TransparentWidget::onDragEnd(e);
    }

    void onHover(const event::Hover &e) override
    {
        TransparentWidget::onHover(e);
        e.consume(this);
    }

    void onHoverKey(const event::HoverKey &e) override
    {
        VoltageSequencer *sequencer = current_sequencer;

        this->shift_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT);
        this->ctrl_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL);

        // Randomize single sequence by hovering over and pressing 'r'

        if (e.key == GLFW_KEY_R && e.action == GLFW_PRESS)
        {
            // Do not randomize if CTRL-r is pressed.  That's for randomizing everything
            if ((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL)
            {
                sequencer->randomizeInWindow();
            }
        }
    }

    void onLeave(const LeaveEvent &e) override
    {
        shift_key = false;
        ctrl_key = false;
        tooltip->deactivateTooltip();
    }

    void onDoubleClick(const event::DoubleClick &e) override
    {
        this->double_click = true;
        e.consume(this);
    }


    void createContextMenu() 
    {
        // Create a new menu
        ui::Menu* menu = createMenu();

        menu->addChild(createMenuItem("Shift Left", "(shift + drag)", [=]() 
        {
            current_sequencer->history_manager.startSession();
            current_sequencer->shiftLeftInWindow();
            current_sequencer->history_manager.endSession();
        }));

        menu->addChild(createMenuItem("Shift Right", "(shift + drag)", [=]() {
            current_sequencer->history_manager.startSession();
            current_sequencer->shiftRightInWindow();
            current_sequencer->history_manager.endSession();
        }));

        menu->addChild(createMenuItem("Randomize", "", [=]() {
            current_sequencer->randomizeInWindow();
        }));

        menu->addChild(new ui::MenuSeparator);

        menu->addChild(createMenuItem("Reverse", "", [=]() {
            current_sequencer->reverseInWindow();
        }));

        menu->addChild(createMenuItem("Shuffle", "", [=]() {
            current_sequencer->shuffleInWindow();
        }));

        menu->addChild(createMenuItem("Invert", "", [=]() {
            current_sequencer->invertInWindow();
        }));

        menu->addChild(createMenuItem("Sort", "", [=]() {
            current_sequencer->sortInWindow();
        }));

        menu->addChild(createMenuItem("Mirror", "", [=]() {
            current_sequencer->mirrorInWindow();
        }));

        menu->addChild(new ui::MenuSeparator);

        menu->addChild(createMenuItem("Reset to Default", "", [=]() {
            current_sequencer->resetInWindow();
        }));

        menu->addChild(createMenuItem("Zero", "", [=]() {
            current_sequencer->zeroInWindow();
        }));


        // Add a separator
        menu->addChild(new ui::MenuSeparator);

        // Add undo/redo menu items
        menu->addChild(createMenuItem("Undo", "", [=]() {
            current_sequencer->undo();
        }));

        menu->addChild(createMenuItem("Redo", "", [=]() {
            current_sequencer->redo();
        }));
    }
};
