struct ArpSeqWindow : TransparentWidget
{
    ArpSeq *module;
    VoltageSequencer *voltage_sequencer;
    VoltageSequencer *chance_sequencer;

    unsigned int double_click_memory_start = 0;
    unsigned int double_click_memory_end = 0;

    enum DRAG_MODES 
    {
        DRAG_FROM_START,
        DRAG_FROM_MIDDLE,
        DRAG_FROM_END
    };

    // variables for drag features
    Vec drag_position;
    int drag_origin_column = 0;
    int drag_origin_start_column = 0;
    int drag_origin_end_column = 0;

    int drag_mode = DRAG_FROM_START; // 0 = drag start, 1 = drag middle, 2 = drag end

    float cell_width = (DRAW_AREA_WIDTH - (((float)MAX_SEQUENCER_STEPS) * BAR_HORIZONTAL_PADDING)) / (float)MAX_SEQUENCER_STEPS;
    float cell_height = 0.0;

    bool shift_key = false;
    bool ctrl_key = false;
    bool draw_horizontal_guide = false;

    int previous_shift_sequence_column = 0;
    int shift_sequence_column = 0;
    int previous_control_sequence_column = 0;
    int control_sequence_column = 0;

    bool double_clicked = false;

    NVGcolor background_color = nvgRGB(168, 142, 13);
    NVGcolor range_handles_color = nvgRGB(255, 215, 20);

    ArpSeqWindow(VoltageSequencer *voltage_sequencer, VoltageSequencer *chance_sequencer)
    {
        this->voltage_sequencer = voltage_sequencer;
        this->chance_sequencer = chance_sequencer;
        
        box.size = Vec(DRAW_AREA_WIDTH, cell_width);
        cell_height = cell_width * 0.6;
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (layer == 1)
        {
            const auto vg = args.vg;

            // Save the drawing context to restore later
            nvgSave(vg);

            // Iterate over each slot in the window
            for (int step = 0; step < MAX_SEQUENCER_STEPS; step++)
            {
                // Get the arp_seq_window_start and arp_seq_window_end from the 

                NVGcolor cell_color = background_color;

                // If step is equal to arp_seq_window_start or arp_seq_window_end, then
                // set the cell color to the sequence_position_highlight_color.
                if (step == voltage_sequencer->window_start || step == voltage_sequencer->window_end)
                {
                    cell_color = range_handles_color;
                }

                // If step is greater than arp_seq_window_start and less than arp_seq_window_end,
                // then draw the cell
                if((step >= voltage_sequencer->window_start) && (step <= voltage_sequencer->window_end))
                {
                    drawCell(vg, step, cell_color);
                }
            }

            nvgRestore(vg);
        }
    }

    void drawCell(NVGcontext *vg, int column, NVGcolor color)
    {
        // the context of the function call nvgRect(vg, x, y, w, h);:
        //
        // vg is the NanoVG context.
        // x and y define the top-left corner of the rectangle.
        // w is the width of the rectangle.
        // h is the height of the rectangle.

        // double height = cell_height;
        // double height = 5.0;
        double x = (column * cell_width) + (column * BAR_HORIZONTAL_PADDING);

        nvgBeginPath(vg);
        nvgFillColor(vg, color);
        nvgRect(vg, x, 0.0, this->cell_width, this->cell_height);
        nvgFill(vg);
    }

    void onDoubleClick(const event::DoubleClick &e) override
    {
        // If window start is zero and window end is MAX_SEQUENCER_STEPS - 1, then
        // set the window start and window end to the values stored in the double_click_memory_start
        // and double_click_memory_end variables.

        if(voltage_sequencer->window_start == 0 && voltage_sequencer->window_end == MAX_SEQUENCER_STEPS - 1)
        {
            voltage_sequencer->setWindowStart(double_click_memory_start);
            voltage_sequencer->setWindowEnd(double_click_memory_end);
            chance_sequencer->setWindowStart(double_click_memory_start);
            chance_sequencer->setWindowEnd(double_click_memory_end);
        }
        else
        {
            // Store the current window start and window end values in the double_click_memory_start
            // and double_click_memory_end variables.
            double_click_memory_start = voltage_sequencer->window_start;
            double_click_memory_end = voltage_sequencer->window_end;

            // double click to reset the window
            voltage_sequencer->setWindowStart(0);
            voltage_sequencer->setWindowEnd(MAX_SEQUENCER_STEPS - 1);
            chance_sequencer->setWindowStart(0);
            chance_sequencer->setWindowEnd(MAX_SEQUENCER_STEPS - 1);
        }

        e.consume(this);
        double_clicked = true;
    }

    void onButton(const event::Button &e) override
    {
        // If left mouse up, then reset double click flag
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE && double_clicked)
        {
            double_clicked = false;
        }

        // If the user double clicked, then don't do anything.
        if(double_clicked)
        {
            return;
        }

        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            e.consume(this);
            TransparentWidget::onButton(e);

            drag_position = e.pos;

            // Determine which column was clicked
            drag_origin_column = getColumnFromMousePosition(e.pos);
            drag_origin_start_column = voltage_sequencer->window_start;
            drag_origin_end_column = voltage_sequencer->window_end;

            // Handle the edge case where the window start and end are set to the
            // same column.

            if (drag_origin_start_column == drag_origin_end_column)
            {
                // If we're at the beginning of the sequence, then allow the user to 
                // extend the window to the right.
                if(drag_origin_start_column == 0)
                { 
                    drag_mode = DRAG_FROM_END;
                }
                // If we're at the end of the sequence, then allow the user to
                // extend the window to the left.
                else if(drag_origin_start_column == MAX_SEQUENCER_STEPS - 1) 
                {
                    drag_mode = DRAG_FROM_START;
                }
                // Otherwise use the prevously set drag_mode value unless it was 1 (shift).
                else
                {
                    if(drag_mode == DRAG_FROM_MIDDLE) drag_mode = DRAG_FROM_END;
                }
            }
            else
            {
                // This is the typical behavior, where the start and end
                // columns are different.
                if(drag_origin_column == voltage_sequencer->window_start)
                {
                    drag_mode = DRAG_FROM_START;
                }
                else if(drag_origin_column == voltage_sequencer->window_end)
                {
                    drag_mode = DRAG_FROM_END;
                }
                else
                {
                    drag_mode = DRAG_FROM_MIDDLE;
                }
            }
        }

        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0)
        {
            // createContextMenu();
            e.consume(this);
        }
    }

    void onDragMove(const event::DragMove &e) override
    {
        if(double_clicked)
        {
            return;
        }

        TransparentWidget::onDragMove(e);
        float zoom = getAbsoluteZoom();
        drag_position = drag_position.plus(e.mouseDelta.div(zoom));

        if(e.button != GLFW_MOUSE_BUTTON_LEFT) return;

        int column = getColumnFromMousePosition(drag_position);

        // unsigned int page = module->page;

        if(drag_mode == 0)
        {   
            voltage_sequencer->setWindowStart(column);
            chance_sequencer->setWindowStart(column);
        }
        if(drag_mode == 1)
        {
            // Compute the difference between the drag origin column and the current column
            int column_delta = column - drag_origin_column;

            //
            // This block of code ensures that the column delta doesn't
            // cause the window to go out of bounds.

            if ((drag_origin_start_column + column_delta) < 0)
            {
                column_delta = 0 - drag_origin_start_column;
            }

            if ((drag_origin_end_column + column_delta) > MAX_SEQUENCER_STEPS - 1)
            {
                column_delta = (MAX_SEQUENCER_STEPS - 1) - drag_origin_end_column;
            }

            int window_start = drag_origin_start_column + column_delta;
            int window_end = drag_origin_end_column + column_delta;

            // Shift both the window start and window end by the column delta
            voltage_sequencer->setWindowStart(window_start);
            voltage_sequencer->setWindowEnd(window_end);

            // Also set the chanceage sequencer window range
            chance_sequencer->setWindowStart(window_start);
            chance_sequencer->setWindowEnd(window_end);
        }
        else if(drag_mode == 2)
        {
            if (column > MAX_SEQUENCER_STEPS - 1)  column = MAX_SEQUENCER_STEPS - 1;

            voltage_sequencer->setWindowEnd(column);
            chance_sequencer->setWindowEnd(column);
        }
    }

    int getColumnFromMousePosition(Vec mouse_position)
    {
        // Determine which column was clicked
        int column = (unsigned int)(mouse_position.x / (cell_width + BAR_HORIZONTAL_PADDING));

        // clamp the column to the range of 0 to MAX_SEQUENCER_STEPS
        clamp(column, 0, MAX_SEQUENCER_STEPS);

        return column;
    }

};