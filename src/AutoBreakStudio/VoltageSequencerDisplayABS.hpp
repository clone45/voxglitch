struct VoltageSequencerDisplayABS : SequencerDisplayABS
{
    AutobreakStudio *module;
    VoltageSequencer **sequencer_ptr_ptr;

    bool shift_key = false;
    bool ctrl_key = false;
    bool draw_horizontal_guide = false;

    int previous_shift_sequence_column = 0;
    int shift_sequence_column = 0;

    int previous_control_sequence_column = 0;
    int control_sequence_column = 0;

    VoltageSequencerDisplayABS(VoltageSequencer **sequencer_instance)
    {
        this->sequencer_ptr_ptr = sequencer_instance;

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
                // Get a pointer to the Voltage Sequencer
                VoltageSequencer *sequencer = *sequencer_ptr_ptr;

                //
                // Display the pattern
                //
                for (unsigned int i = 0; i < MAX_SEQUENCER_STEPS; i++)
                {
                    value = sequencer->getValue(i);

                    // Draw grey background bar
                    if (i < sequencer->getLength())
                    {
                        bar_color = brightness(bright_background_color, settings::rackBrightness);
                    }
                    else
                    {
                        bar_color = brightness(dark_background_color, settings::rackBrightness);
                    }

                    drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, bar_color); // background

                    if (i == sequencer->getPlaybackPosition())
                    {
                        bar_color = current_step_highlight_color;
                    }
                    else if (i < sequencer->getLength())
                    {
                        bar_color = lesser_step_highlight_color;
                    }
                    else
                    {
                        bar_color = default_step_highlight_color;
                    }

                    // Draw bars for the sequence values
                    if (value > 0) drawBar(vg, i, (value * DRAW_AREA_HEIGHT), DRAW_AREA_HEIGHT, bar_color);

                    // Highlight the sequence playback column
                    if (i == sequencer->getPlaybackPosition())
                    {
                        drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, sequence_position_highlight_color);
                    }
                }
            }
            else // Draw a demo sequence so that the sequencer looks nice in the library selector
            {
                double demo_sequence[16] = {0.0, 0.0, 0.25,0.75,0.50,0.50,0.25,0.75,0.0,0.0,0.25,0.75,0.0,0.0,0.25,.75};

                for (unsigned int i = 0; i < MAX_SEQUENCER_STEPS; i++)
                {
                    float value = demo_sequence[i];

                    // Draw blue background bars
                    drawBar(vg, i, BAR_HEIGHT, DRAW_AREA_HEIGHT, bright_background_color);

                    // Draw bar for value at i
                    if (value > 0) drawBar(vg, i, (value * DRAW_AREA_HEIGHT), DRAW_AREA_HEIGHT, lesser_step_highlight_color);

                    // Highlight active step
                    if (i == 3) drawBar(vg, i, DRAW_AREA_HEIGHT, DRAW_AREA_HEIGHT, nvgRGBA(255, 255, 255, 150));
                }
            }

            drawVerticalGuildes(vg, DRAW_AREA_HEIGHT);
            drawOverlay(vg, OVERLAY_WIDTH, DRAW_AREA_HEIGHT);
            if(draw_horizontal_guide) drawHorizontalGuide(vg);

            nvgRestore(vg);
        }
    }

    void drawHorizontalGuide(NVGcontext *vg)
    {
        // This calculation for y takes advance of the fact that all
        // ranges that would have the 0-guide visible are symmetric, so
        // it will need updating if non-symmetric ranges are added.
        double y = DRAW_AREA_HEIGHT / 2.0;

        nvgBeginPath(vg);
        nvgRect(vg, 1, y, (DRAW_AREA_WIDTH - 2), 1.0);
        nvgFillColor(vg, nvgRGBA(240, 240, 255, 40));
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
        if (module)
        {
            VoltageSequencer *sequencer = *sequencer_ptr_ptr;

            double bar_width = (DRAW_AREA_WIDTH / (double) MAX_SEQUENCER_STEPS) - BAR_HORIZONTAL_PADDING;
            int clicked_bar_x_index = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
            int clicked_y = DRAW_AREA_HEIGHT - mouse_position.y;

            clicked_bar_x_index = clamp(clicked_bar_x_index, 0, MAX_SEQUENCER_STEPS - 1);
            clicked_y = clamp(clicked_y, 0, (int) DRAW_AREA_HEIGHT);

            // convert the clicked_y position to a double between 0 and 1
            double value = (double)clicked_y / (double)DRAW_AREA_HEIGHT;

            sequencer->setValue(clicked_bar_x_index, value);
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
        VoltageSequencer *sequencer = *sequencer_ptr_ptr;

        int clicked_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
        clicked_column = clamp(clicked_column, 0, MAX_SEQUENCER_STEPS);
        sequencer->setLength(clicked_column);
    }

    void dragShiftSequences(Vec mouse_position)
    {
        if (module)
        {
            VoltageSequencer *sequencer = *sequencer_ptr_ptr;

            int drag_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
            int shift_offset = drag_column - this->shift_sequence_column;

            while (shift_offset < 0)
            {
                sequencer->shiftLeft();
                shift_offset++;
            }

            while (shift_offset > 0)
            {
                sequencer->shiftRight();
                shift_offset--;
            }

            this->shift_sequence_column = drag_column;
        }
    }

    void dragControlSequence(Vec mouse_position)
    {
        if (module)
        {
            VoltageSequencer *sequencer = *sequencer_ptr_ptr;

            int drag_column = mouse_position.x / (bar_width + BAR_HORIZONTAL_PADDING);
            drag_column = clamp(drag_column, 0, MAX_SEQUENCER_STEPS);
            sequencer->setLength(drag_column);
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
        VoltageSequencer *sequencer = *sequencer_ptr_ptr;

        this->shift_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT);
        this->ctrl_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL);

        // Randomize single sequence by hovering over and pressing 'r'

        if (e.key == GLFW_KEY_R && e.action == GLFW_PRESS)
        {
            // Do not randomize if CTRL-r is pressed.  That's for randomizing everything
            if ((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL)
            {
                sequencer->randomize();
            }
        }
    }

    void onLeave(const LeaveEvent &e) override
    {
        shift_key = false;
        ctrl_key = false;
    }
};
