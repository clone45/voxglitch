struct GrooveboxExpanderMuteButton : GrooveboxExpanderSoftButton
{
    struct UnmuteAllMenuItem : MenuItem
    {
        GrooveBoxExpander *module;

        void onAction(const event::Action &e) override
        {
            module->unmuteAll();
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        GrooveBoxExpander *module = dynamic_cast<GrooveBoxExpander *>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator);

        UnmuteAllMenuItem *unmute_all = createMenuItem<UnmuteAllMenuItem>("Unmute All");
        unmute_all->module = module;
        menu->addChild(unmute_all);
    }
};