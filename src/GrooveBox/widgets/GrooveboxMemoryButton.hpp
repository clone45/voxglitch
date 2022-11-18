struct GrooveboxMemoryButton : GrooveboxSoftButton
{
    unsigned int memory_slot = 0;

    void onButton(const event::Button &e) override
    {
        // Don't allow the user to toggle the activly selected memory button
        // Memory buttons aren't meant to be toggled.  They're used more like
        // a radio button in HTML.

        if (!(module->memory_slot_index == this->memory_slot))
        {
            SvgSwitch::onButton(e);
        }
    }
};
