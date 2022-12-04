struct GrooveboxExpanderSoloButton : GrooveboxExpanderSoftButtonGreen
{
    unsigned int track_index = 0;

    struct ExclusiveSoloMenuItem : MenuItem
    {
        GrooveBoxExpander *module;
        unsigned int track_index = 0;

        void onAction(const event::Action &e) override
        {
            module->exclusiveSolo(track_index);
        }
    };

    struct UnsoloAllMenuItem : MenuItem
    {
        GrooveBoxExpander *module;

        void onAction(const event::Action &e) override
        {
            module->unsoloAll();
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        GrooveBoxExpander *module = dynamic_cast<GrooveBoxExpander *>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator);

        ExclusiveSoloMenuItem *exclusive_solo_menu_item = createMenuItem<ExclusiveSoloMenuItem>("Exclusive Solo");
        exclusive_solo_menu_item->module = module;
        exclusive_solo_menu_item->track_index = track_index;
        menu->addChild(exclusive_solo_menu_item);

        UnsoloAllMenuItem *unsolo_all_menu_item = createMenuItem<UnsoloAllMenuItem>("Unsolo All");
        unsolo_all_menu_item->module = module;
        menu->addChild(unsolo_all_menu_item);        
    }
};