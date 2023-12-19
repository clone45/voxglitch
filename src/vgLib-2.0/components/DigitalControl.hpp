/*
 * Apology & Explanation:
 * 
 * In the development of this module, certain design trade-offs were made for expediency. Some parts of the code, 
 * especially where UI state is directly tied to the module state, are not aligned with best practices. The optimal
 * approach would involve a more decoupled design, allowing UI components and the core logic to interact through well-defined
 * interfaces, enhancing maintainability and scalability.
 * 
 * Taking a deeper look, by directly linking the UI state to the internal module state, I've introduced tight coupling 
 * that could make changes and extensions to the codebase challenging in the future. Ideally, adopting patterns such as MVC 
 * (Model-View-Controller) or MVVM (Model-View-ViewModel) could allow for better separation of concerns. This would streamline 
 * code readability, facilitate unit testing, and make the introduction of new features or modifications less error-prone.
 * 
 * I recognize this shortcoming and plan to refine this in future iterations. Thanks for understanding.
 *
 */

struct DigitalControl : Widget 
{
    std::string label_text_on;
    std::string label_text_off;

    NVGcolor label_color = nvgRGB(234, 202, 63);
    Widget* control;
    Module* module;
    float corner_radius = 2.5f;
    float bg_height = 18.0f;
    float bg_width;
    float text_width = 0.0f;
    bool hover = false;

    DigitalControl(Vec pos, std::string label_text_on, std::string label_text_off, Widget* control_widget, Module* module) 
    {
        this->box.pos = pos;
        this->label_text_on = label_text_on;
        this->label_text_off = label_text_off;
        this->module = module;

        control = control_widget;
        addChild(control);
    }

    void calculateDimensions(NVGcontext* vg) 
    {
        float right_padding = 8.0;
        float left_text_padding = 8.0;
        float right_text_padding = 5.0;

        // This is necessary to calculate the text width properly
        setFontAttributes(vg);

        float bounds[4];
        nvgTextBounds(vg, 0, 0, label_text_on.c_str(), NULL, bounds);
        text_width = bounds[2] - bounds[0];

        // Adjust the control widget position (x-coordinate)
        float control_x = left_text_padding + text_width + right_text_padding;
        control->box.pos.x = control_x;

        // Adjust the control widget position (y-coordinate)
        float control_y = (bg_height - control->box.size.y) / 2;
        control->box.pos.y = control_y;

        // Adjust the width of the background
        bg_width = control_x + control->box.size.x + right_padding;

        // Set the bounding box of this widget
        box.size.x = bg_width;
        box.size.y = bg_height;
    }

    void draw(const DrawArgs &args) override 
    {
        //
        // Draw the background first
        //

        NVGcolor background_color;

        // set a variable to hold the color nvgRGB(49, 42, 9)
        if(hover)
        {
            background_color = nvgRGB(65, 53, 1);
        }
        else
        {
            background_color = nvgRGB(49, 42, 9);
        }
        

        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, bg_width, bg_height, corner_radius);
        nvgFillColor(args.vg, background_color);  // RGBA color for the background
        nvgFill(args.vg);

        std::string label_text;

        if(dynamic_cast<DigitalToggle*>(control) != nullptr) 
        {
            DigitalToggle* digital_toggle = dynamic_cast<DigitalToggle*>(control);
            label_text = digital_toggle->getState() ? label_text_off : label_text_on;
        }
        if(dynamic_cast<DigitalSwitch*>(control) != nullptr) 
        {
            DigitalSwitch* digital_switch = dynamic_cast<DigitalSwitch*>(control);
            label_text = digital_switch->getState() ? label_text_off : label_text_on;
        } 
        if(dynamic_cast<DigitalSelect*>(control) != nullptr) 
        {
            label_text = "Scale: ";
        } 
        // Obviously this will need rethinking as soon as we have more than one slider
        // with more than one purpose.
        if(dynamic_cast<DigitalHorizontalSlider*>(control) != nullptr) 
        {
            label_text = "Slew: ";
        } 
        if(dynamic_cast<DigitalRangeSelector*>(control) != nullptr) 
        {
            // Decide on the label for the range selector if necessary.
            DigitalRangeSelector* range = dynamic_cast<DigitalRangeSelector*>(control);

            float display_start_range;
            float display_end_range;

            if(*range->polarity == false) // unipolar
            {
                display_start_range = range->getStartRange() * 10;
                display_end_range = range->getEndRange() * 10;
            }
            else // bipolar should display from -5.00 to 5.00
            {
                display_start_range = (range->getStartRange() * 10) - 5;
                display_end_range = (range->getEndRange() * 10) - 5;
            }

            char startRangeStr[10];
            char endRangeStr[10];
            sprintf(startRangeStr, "%.2f", display_start_range);
            sprintf(endRangeStr, "%.2f", display_end_range);
            label_text = "Range: " + std::string(startRangeStr) + " to " + std::string(endRangeStr) + "V";
        }

        //
        // Draw the label text
        //

        setFontAttributes(args.vg);
        nvgFillColor(args.vg, label_color);
        nvgText(args.vg, 8, bg_height / 2, label_text.c_str(), NULL);  // Vertically center the text within the background

        // Let the base class handle drawing children
        Widget::draw(args);
    }

    void step() override 
    {
        if (text_width == 0.0f) {  // Ensures the calculation happens only once
            calculateDimensions(APP->window->vg);
        }
        Widget::step();
    }

    void setFontAttributes(NVGcontext* vg) 
    {
        nvgFontSize(vg, 12);
        nvgFontFaceId(vg, 0);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    }

    void onButton(const event::Button &e) override 
    {
        // Create a new event with the same properties as the original
        event::Button new_event = e;

        // Modify the event to be relative to the control widget
        new_event.pos.x -= control->box.pos.x;
        new_event.pos.y -= control->box.pos.y;

        // Forward the event to the control widget
        control->onButton(new_event);
    }

    void onDragMove(const event::DragMove &e) override
    {
        // Forward the event to the control widget
        control->onDragMove(e);
    }

    void onDragEnd(const event::DragEnd &e) override
    {
        // Forward the event to the control widget
        control->onDragEnd(e);
    }

    void onHover(const event::Hover &e) override
    {
        hover = true;

        // Forward the event to the control widget
        control->onHover(e);
        e.consume(this);
    }

    void onLeave(const event::Leave &e) override
    {
        hover = false;

        // Forward the event to the control widget
        control->onLeave(e);
        e.consume(this);
    }

};
