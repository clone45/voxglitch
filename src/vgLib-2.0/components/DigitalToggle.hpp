struct DigitalToggle : SvgWidget
{
    bool *state = nullptr;
    bool old_state = false;

    std::string svg_on = "res/modules/arpseq/readout/QuantizeOn.svg";
    std::string svg_off = "res/modules/arpseq/readout/QuantizeOff.svg";

    DigitalToggle(bool *state)
    {
        this->state = state;
        this->old_state = *state;

        box.size = Vec(10.0, 10.0);

        // Ensure the initial SVG reflects the initial state
        updateSvg();
    }

    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            // Toggle the state and update the SVG
            this->toggleState();

            e.consume(this);
        }
    }

    bool getState() const
    {
        return *state;
    }

    void setState(bool new_state)
    {
        // Assign the new state through the pointer
        *state = new_state;

        // Update the SVG based on the new state
        updateSvg();
    }

    bool toggleState()
    {
        // Toggle and update the state through the pointer
        this->setState(!this->getState());

        return this->getState();
    }

    void updateSvg()
    {
        // Depending on the state, set the SVG
        if (this->getState())
        {
            setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, svg_on)));
        }
        else
        {
            setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, svg_off)));
        }
    }

    void step() override
    {
        // If the state has changed, update the SVG
        if (old_state != *state)
        {
            updateSvg();
            old_state = *state;
        }

        SvgWidget::step();
    }
};