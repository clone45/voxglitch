struct DigitalRangeSelector : Widget
{
    float WIDTH = 100.0f;
    float HEIGHT = 10.0f;

    float *start_range; // Represented as a fraction of total width, e.g., 0.2 for 20% from the left
    float *end_range;   // Similarly, e.g., 0.8 for 80% from the left
    bool *polarity = nullptr;

    // Dimensions and positions
    Rect track_rectangle;      // Rectangle representing the track
    Rect left_thumb_rectangle;  // Rectangle representing the left thumb
    Rect right_thumb_rectangle; // Rectangle representing the right thumb

    // Flags to check if thumbs are being dragged
    bool is_left_thumb_dragged = false;
    bool is_right_thumb_dragged = false;

    Vec drag_position;

    DigitalRangeSelector(bool *polarity, float *start_range, float *end_range)
    {
        this->start_range = start_range;
        this->end_range = end_range;
        this->polarity = polarity;

        // Initialize the track's dimensions and positions based on WIDTH
        track_rectangle.size.x = WIDTH;
        track_rectangle.size.y = HEIGHT;
        track_rectangle.pos = Vec(0, (HEIGHT - track_rectangle.size.y) / 2); // Vertically center the track in the widget

        // Initialize the thumbs' dimensions and positions based on start_range and end_range
        left_thumb_rectangle.size.x = 4;
        left_thumb_rectangle.size.y = HEIGHT;
        left_thumb_rectangle.pos.x = track_rectangle.pos.x + (*start_range * WIDTH);

        right_thumb_rectangle.size.x = 4;
        right_thumb_rectangle.size.y = HEIGHT;
        right_thumb_rectangle.pos.x = (track_rectangle.pos.x + *end_range * WIDTH) - right_thumb_rectangle.size.x;

        this->box.size.x = WIDTH;
        this->box.size.y = HEIGHT;
    }

    void draw(const DrawArgs &args) override
    {
        right_thumb_rectangle.pos.x = (getEndRange() * WIDTH) - right_thumb_rectangle.size.x;
        left_thumb_rectangle.pos.x = getStartRange() * WIDTH;

        // Drawing the track
        nvgBeginPath(args.vg);
        nvgRect(args.vg, track_rectangle.pos.x, track_rectangle.pos.y, track_rectangle.size.x, track_rectangle.size.y);
        nvgFillColor(args.vg, nvgRGB(94, 78, 7)); // Light gray for the track
        nvgFill(args.vg);

        // Drawing a line between the two thumbs
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, left_thumb_rectangle.pos.x + left_thumb_rectangle.size.x, left_thumb_rectangle.pos.y + left_thumb_rectangle.size.y / 2);
        nvgLineTo(args.vg, right_thumb_rectangle.pos.x, right_thumb_rectangle.pos.y + right_thumb_rectangle.size.y / 2);
        nvgStrokeColor(args.vg, nvgRGB(255, 215, 20)); // Using the same color as the thumbs
        nvgStrokeWidth(args.vg, 1.0); // Width of the line
        nvgStroke(args.vg);

        // Drawing the left thumb
        nvgBeginPath(args.vg);
        nvgRect(args.vg, left_thumb_rectangle.pos.x, left_thumb_rectangle.pos.y, left_thumb_rectangle.size.x, left_thumb_rectangle.size.y);
        nvgFillColor(args.vg, nvgRGB(255, 215, 20)); // Dark gray for the thumbs
        nvgFill(args.vg);

        // Drawing the right thumb
        nvgBeginPath(args.vg);
        nvgRect(args.vg, right_thumb_rectangle.pos.x, right_thumb_rectangle.pos.y, right_thumb_rectangle.size.x, right_thumb_rectangle.size.y);
        nvgFillColor(args.vg, nvgRGB(255, 215, 20)); // Dark gray for the thumbs
        nvgFill(args.vg);
    }

    void onButton(const event::Button &e) override
    {

        // Check if the left mouse button was pressed
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            e.consume(this); // Mark the event as consumed

            drag_position = e.pos;

            // Check if the click happened within the bounds of the left thumb
            if (thumbContains(left_thumb_rectangle, e.pos))
            {
                is_left_thumb_dragged = true;
                is_right_thumb_dragged = false; // Just to ensure no ambiguity
            }
            // Check if the click happened within the bounds of the right thumb
            else if (thumbContains(right_thumb_rectangle, e.pos))
            {
                is_right_thumb_dragged = true;
                is_left_thumb_dragged = false; // Again, just to ensure no ambiguity
            }

            // Check if the click happened to the right of the right thumb but still within the control
            else if (e.pos.x > right_thumb_rectangle.pos.x + right_thumb_rectangle.size.x && e.pos.x <= WIDTH)
            {
                right_thumb_rectangle.pos.x = e.pos.x - right_thumb_rectangle.size.x / 2;
                setEndRange((right_thumb_rectangle.pos.x + right_thumb_rectangle.size.x) / WIDTH);
                is_right_thumb_dragged = true;  // Set this to true
                is_left_thumb_dragged = false;  // Ensure no ambiguity
            }
            // Check if the click happened to the left of the left thumb but still within the control
            else if (e.pos.x < left_thumb_rectangle.pos.x && e.pos.x >= 0)
            {
                left_thumb_rectangle.pos.x = e.pos.x;
                setStartRange(left_thumb_rectangle.pos.x / WIDTH);
                is_left_thumb_dragged = true;   // Set this to true
                is_right_thumb_dragged = false; // Ensure no ambiguity
            }

            // If the click is between the two thumbs
            else if (e.pos.x > left_thumb_rectangle.pos.x + left_thumb_rectangle.size.x && e.pos.x < right_thumb_rectangle.pos.x)
            {
                float leftDistance = e.pos.x - (left_thumb_rectangle.pos.x + left_thumb_rectangle.size.x);
                float rightDistance = right_thumb_rectangle.pos.x - e.pos.x;

                // If the click is closer to the left thumb
                if (leftDistance < rightDistance)
                {
                    left_thumb_rectangle.pos.x = e.pos.x;
                    setStartRange(left_thumb_rectangle.pos.x / WIDTH);
                    is_left_thumb_dragged = true;   // Set this to true
                    is_right_thumb_dragged = false; // Ensure no ambiguity
                }
                // If the click is closer to the right thumb
                else
                {
                    right_thumb_rectangle.pos.x = e.pos.x - right_thumb_rectangle.size.x;
                    setEndRange((right_thumb_rectangle.pos.x + right_thumb_rectangle.size.x) / WIDTH);
                    is_right_thumb_dragged = true;  // Set this to true
                    is_left_thumb_dragged = false;  // Ensure no ambiguity
                }
            }
        }

        // onLeftThumbChange(start_range);
        // onRightThumbChange(end_range);
    }

    void onDragMove(const event::DragMove &e) override
    {
        float zoom = getAbsoluteZoom();
        drag_position = drag_position.plus(e.mouseDelta.div(zoom));

        if (is_left_thumb_dragged)
        {
            float proposed_x = drag_position.x - left_thumb_rectangle.size.x / 2;
            proposed_x = clamp(proposed_x, 0.f, right_thumb_rectangle.pos.x - left_thumb_rectangle.size.x);

            // Translate this to the start_range value
            float proposed_start_range = proposed_x / WIDTH;

            // Snap it to the nearest 0.025 increment
            setStartRange(round(proposed_start_range * 40.0f) / 40.0f);
            // left_thumb_rectangle.pos.x = getStartRange() * WIDTH;
        }
        else if (is_right_thumb_dragged)
        {
            float proposed_x = drag_position.x - right_thumb_rectangle.size.x / 2;
            proposed_x = clamp(proposed_x, left_thumb_rectangle.pos.x + left_thumb_rectangle.size.x, WIDTH - right_thumb_rectangle.size.x);

            // Translate this to the end_range value and adjust for the width of the thumb
            float proposed_end_range = (proposed_x + right_thumb_rectangle.size.x) / WIDTH;

            // Snap it to the nearest 0.025 increment
            setEndRange(round(proposed_end_range * 40.0f) / 40.0f);
            // right_thumb_rectangle.pos.x = (getEndRange() * WIDTH) - right_thumb_rectangle.size.x;
        }
        Widget::onDragMove(e);
    }

    void onDragEnd(const event::DragEnd &e) override
    {
        // Reset the flags
        is_left_thumb_dragged = false;
        is_right_thumb_dragged = false;
    }

    void setStartRange(float range)
    {
        *start_range = range;
    }

    float getStartRange()
    {
        return *start_range;
    }

    void setEndRange(float range)
    {
        *end_range = range;
    }

    float getEndRange()
    {
        return *end_range;
    }

    bool thumbContains(Rect thumb, Vec pos)
    {
        // Return true if the position is within the horizontal bounds of the thumb
        // Allow the position to be outsize the vertical bounds of the thumb to give more room for dragging
        return pos.x >= thumb.pos.x && pos.x <= thumb.pos.x + thumb.size.x;
    }
};
