struct GrooveboxMemoryButton : VCVLightLatch<MediumSimpleLight<GreenLight>>
{
    unsigned int memory_slot = 0;
    GrooveBox *module;

    void onButton(const event::Button &e) override
    {
        // Don't allow the user to toggle the activly selected memory button
        // Memory buttons aren't meant to be toggled.  They're used more like
        // a radio button in HTML.

        if (!(module->memory_slot_index == this->memory_slot) && (!module->memCableIsConnected()))
        {
            SvgSwitch::onButton(e);
        }
    }
};
