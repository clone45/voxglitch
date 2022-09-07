struct VoltageSequencerDisplayABS : SequencerDisplayABS
{
    AutobreakStudio *module;

    bool shift_key = false;
    bool ctrl_key = false;

    int previous_shift_sequence_column = 0;
    int shift_sequence_column = 0;

    int previous_control_sequence_column = 0;
    int control_sequence_column = 0;

    bool logged = false;

    VoltageSequencerDisplayABS()
    {
        // The bounding box needs to be a little deeper than the visual
        // controls to allow mouse drags to indicate '0' (off) column heights,
        // which is why 16 is being added to the draw height to define the
        // bounding box.
        box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT + 16);
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            const auto vg = args.vg;
            double value;
            NVGcolor bar_color;

            // Save the drawing context to restore later
            nvgSave(vg);

            if(module)
            {
                //
                // Display the pattern
                //
                for (unsigned int i = 0; i < MAX_SEQUENCER_STEPS; i++)
                {
                    value = module->selected_voltage_sequencer->getValue(i);

                    // Draw grey background bar
                    if (i < module->selected_voltage_sequencer->getLength())
                    {
                        bar_color = brightness(bright_background_color, settings::rackBrightness);
                    }
                    else
                    {
                        bar_color = brightness(dark_background_color, settings::rackBrightness);
                    }

                    drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, bar_color); // background

                    if (i == module->selected_voltage_sequencer->getPlaybackPosition())
                    {
                        bar_color = current_step_highlight_color;
                    }
                    else if (i < module->selected_voltage_sequencer->getLength())
                    {
                        bar_color = lesser_step_highlight_color;
                    }
                    else
                    {
                        bar_color = default_step_highlight_color;
                    }

                    // Draw bars for the sequence values
                    if (value > 0)
                        drawBar(vg, i, (value * DRAW_AREA_HEIGHT), DRAW_AREA_HEIGHT, bar_color);

                    // Highlight the sequence playback column
                    if (i == module->selected_voltage_sequencer->getPlaybackPosition())
                    {
                        drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, sequence_position_highlight_color);
                    }
                }
            }
            else // Draw a demo sequence so that the sequencer looks nice in the library selector
            {
                double demo_sequence[32] = {100.0, 100.0, 93.0, 80.0, 67.0, 55.0, 47.0, 44.0, 43.0, 44.0, 50.0, 69.0, 117.0, 137.0, 166, 170, 170, 164, 148, 120, 105, 77, 65, 41, 28, 23, 22, 22, 28, 48, 69, 94};

                for (unsigned int i = 0; i < MAX_SEQUENCER_STEPS; i++)
                {

                    // Draw blue background bars
                    drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, nvgRGBA(60, 60, 64, 255));

                    // Draw bar for value at i
                    drawBar(vg, i, demo_sequence[i], DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 150));

                    // Highlight active step
                    if (i == 5) drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 20));
                }
            }

            drawVerticalGuildes(vg, DRAW_AREA_HEIGHT);
            drawOverlay(vg, OVERLAY_WIDTH, DRAW_AREA_HEIGHT);

            nvgRestore(vg);
        }
    }

    //
    // void editBar(Vec mouse_position)
    //
    // Called when the user clicks to edit one of the sequencer values.  Sets
    // the sequencer value that the user has selected.
    //
    void editBar(Vec mouse_position)
    {
        if (module)
        {
            double bar_width = (DRAW_AREA_WIDTH / MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
            int clicked_bar_x_index = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
            int clicked_y = DRAW_AREA_HEIGHT - mouse_position.y;

            clicked_bar_x_index = clamp(clicked_bar_x_index, 0, MAX_SEQUENCER_STEPS - 1);
            clicked_y = clamp(clicked_y, 0, DRAW_AREA_HEIGHT);

            // convert the clicked_y position to a double between 0 and 1
            double value = (double)clicked_y / (double)DRAW_AREA_HEIGHT;

            module->selected_voltage_sequencer->setValue(clicked_bar_x_index, value);
        }
    }

    void startShiftSequences(Vec mouse_position)
    {
        int clicked_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
        this->previous_shift_sequence_column = clicked_column;
        this->shift_sequence_column = clicked_column;
    }

    void startControlSequence(Vec mouse_position)
    {
        int clicked_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
        clicked_column = clamp(clicked_column, 0, MAX_SEQUENCER_STEPS);
        module->selected_voltage_sequencer->setLength(clicked_column);
    }

    void dragShiftSequences(Vec mouse_position)
    {
        if (module)
        {
            int drag_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
            int shift_offset = drag_column - this->shift_sequence_column;

            while (shift_offset < 0)
            {
                module->selected_voltage_sequencer->shiftLeft();
                shift_offset++;
            }

            while (shift_offset > 0)
            {
                module->selected_voltage_sequencer->shiftRight();
                shift_offset--;
            }

            this->shift_sequence_column = drag_column;
        }
    }

    void dragControlSequence(Vec mouse_position)
    {
        if (module)
        {
            int drag_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
            drag_column = clamp(drag_column, 0, MAX_SEQUENCER_STEPS);
            module->selected_voltage_sequencer->setLength(drag_column);
        }
    }

    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            e.consume(this);
            drag_position = e.pos;

            if (this->shift_key == true)
            {
                startShiftSequences(drag_position);
            }
            else if (this->ctrl_key == true)
            {
                startControlSequence(drag_position);
            }
            else
            {
                this->editBar(e.pos);
            }
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
            dragControlSequence(drag_position);
        }
        else
        {
            editBar(drag_position);
        }
    }

    void onHover(const event::Hover &e) override
    {
        TransparentWidget::onHover(e);
        e.consume(this);
    }

    void onHoverKey(const event::HoverKey &e) override
    {
        if (!module)
            return;

        this->shift_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT);
        this->ctrl_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL);

        // Randomize single sequence by hovering over and pressing 'r'

        if (e.key == GLFW_KEY_R && e.action == GLFW_PRESS)
        {
            // Do not randomize if CTRL-r is pressed.  That's for randomizing everything
            if ((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL)
            {
                module->selected_voltage_sequencer->randomize();
            }
        }
    }

    void onLeave(const LeaveEvent &e) override
    {
        shift_key = false;
        ctrl_key = false;
    }
};
