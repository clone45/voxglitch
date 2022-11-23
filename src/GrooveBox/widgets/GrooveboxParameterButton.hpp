struct GrooveboxParameterButton : GrooveboxSoftButton
{
    unsigned int parameter_index = 0;
    unsigned int parameter_slot = 0;

    void onButton(const event::Button &e) override
    {
        // Don't allow the user to toggle the activly selected parameter button
        // Parameter buttons aren't meant to be toggled.  They're used more like
        // a radio button in HTML.

        if (!(module->selected_parameter_slot == this->parameter_slot))
        {
            SvgSwitch::onButton(e);
        }
    }
};
