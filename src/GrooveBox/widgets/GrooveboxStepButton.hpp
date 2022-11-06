struct GrooveboxStepButton : SvgSwitch
{
    GrooveBox *module;
    bool is_moused_over = false;
    Vec drag_position;
    unsigned int index = 0;

    GrooveboxStepButton()
    {
        momentary = false;
        shadow->opacity = 0;

        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_step_button.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_step_button_lit.svg")));

        box.size = Vec(31.004, 31.004); // was 15.5   (19.28)
    }

    void draw(const DrawArgs &args) override
    {
        SvgSwitch::draw(args);
    }

    struct CopyMenuItem : MenuItem
    {
        GrooveBox *module;

        void onAction(const event::Action &e) override
        {
            // do the copy action
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        GrooveBox *module = dynamic_cast<GrooveBox *>(this->module);
        assert(module);

        // menu->addChild(new MenuEntry); // For spacing only
        // menu->addChild(createMenuLabel("GrooveBox"));
        CopyMenuItem *copy_menu_item = createMenuItem<CopyMenuItem>("Copy");
        copy_menu_item->module = module;
        menu->addChild(copy_menu_item);
    }
};
