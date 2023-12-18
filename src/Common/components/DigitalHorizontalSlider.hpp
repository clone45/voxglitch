struct DigitalHorizontalSlider : Widget
{
    float WIDTH = 74.0f;
    float HEIGHT = 10.0f;

    float *value; // Represents the slider's current value as a fraction of total width

    // Dimensions and positions
    Rect track_rectangle;
    Rect thumb_rectangle;

    // Flag to check if the thumb is being dragged
    bool is_thumb_dragged = false;

    Vec drag_position;

    DigitalHorizontalSlider(float *value)
    {
        this->value = value;

        // Initialize the track's dimensions and positions based on WIDTH
        track_rectangle.size.x = WIDTH;
        track_rectangle.size.y = HEIGHT;
        track_rectangle.pos = Vec(0, (HEIGHT - track_rectangle.size.y) / 2); 

        // Initialize the thumb's dimensions and position
        thumb_rectangle.size.x = 4;
        thumb_rectangle.size.y = HEIGHT;
        thumb_rectangle.pos.x = track_rectangle.pos.x + (*value * WIDTH);

        this->box.size.x = WIDTH;
        this->box.size.y = HEIGHT;
    }

    void draw(const DrawArgs &args) override
    {
        thumb_rectangle.pos.x = getValue() * WIDTH;

        // Drawing the track
        nvgBeginPath(args.vg);
        nvgRect(args.vg, track_rectangle.pos.x, track_rectangle.pos.y, track_rectangle.size.x, track_rectangle.size.y);
        nvgFillColor(args.vg, nvgRGB(94, 78, 7));
        nvgFill(args.vg);

        // Drawing the thumb
        nvgBeginPath(args.vg);
        nvgRect(args.vg, thumb_rectangle.pos.x, thumb_rectangle.pos.y, thumb_rectangle.size.x, thumb_rectangle.size.y);
        nvgFillColor(args.vg, nvgRGB(255, 215, 20));
        nvgFill(args.vg);
    }

    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            e.consume(this);

            drag_position = e.pos;

            if (thumb_rectangle.contains(e.pos))
            {
                is_thumb_dragged = true;
            }
            else if (e.pos.x >= 0 && e.pos.x <= WIDTH)
            {
                thumb_rectangle.pos.x = e.pos.x - thumb_rectangle.size.x / 2;
                setValue(thumb_rectangle.pos.x / WIDTH);
                is_thumb_dragged = true;
            }
        }
    }

    void onDragMove(const event::DragMove &e) override
    {
        float zoom = getAbsoluteZoom();
        drag_position = drag_position.plus(e.mouseDelta.div(zoom));

        if (is_thumb_dragged)
        {
            float proposed_x = drag_position.x - thumb_rectangle.size.x / 2;
            proposed_x = clamp(proposed_x, 0.f, WIDTH - thumb_rectangle.size.x);

            // Translate this to the value and adjust for the width of the thumb
            float proposed_value = proposed_x / WIDTH;

            setValue(proposed_value);
        }

        Widget::onDragMove(e);
    }

    void setValue(float val)
    {
        *value = val;
    }

    float getValue()
    {
        return *value;
    }

    void onDragEnd(const event::DragEnd &e) override
    {
        is_thumb_dragged = false;
    }
};
