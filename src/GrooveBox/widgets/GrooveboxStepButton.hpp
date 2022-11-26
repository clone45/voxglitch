struct GrooveboxStepButton : SvgSwitch
{
    GrooveBox *module;
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

    struct ClearLocksMenuItem : MenuItem
    {
        GrooveBox *module;
        unsigned int index;

        void onAction(const event::Action &e) override
        {
            module->clearStepParameters(index);
        }
    };

    struct CopyMenuItem : MenuItem
    {
        GrooveBox *module;
        unsigned int index;

        void onAction(const event::Action &e) override
        {
           module->copied_step_index = this->index;
        }
    };

    struct PasteMenuItem : MenuItem
    {
        GrooveBox *module;
        unsigned int index;

        void onAction(const event::Action &e) override
        {
            module->pasteStep(this->index);
        }
    };

    struct ShiftRightMenuItem : MenuItem
    {
        GrooveBox *module;

        void onAction(const event::Action &e) override
        {
            module->shiftTrack(-1);
        }
    };

    struct ShiftLeftMenuItem : MenuItem
    {
        GrooveBox *module;

        void onAction(const event::Action &e) override
        {
            module->shiftTrack(1);
        }
    };

    struct RandomizeStepsMenuItem : MenuItem
    {
        GrooveBox *module;

        void onAction(const event::Action &e) override
        {
            module->randomizeSteps();
        }
    };

    struct ClearStepsMenuItem : MenuItem
    {
        GrooveBox *module;

        void onAction(const event::Action &e) override
        {
            module->clearSteps();
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        GrooveBox *module = dynamic_cast<GrooveBox *>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator);

        // Clear all parameter locks for this step
        ClearLocksMenuItem *clear_locks_menu_item = createMenuItem<ClearLocksMenuItem>("Clear All Parameter Locks for this Step");
        clear_locks_menu_item->module = module;
        clear_locks_menu_item->index = index;
        menu->addChild(clear_locks_menu_item);

        menu->addChild(new MenuSeparator);

        // Copy Step Menu Item
        CopyMenuItem *copy_menu_item = createMenuItem<CopyMenuItem>("Copy Step");
        copy_menu_item->module = module;
        copy_menu_item->index = index;
        menu->addChild(copy_menu_item);

        // Paste step menu item
        PasteMenuItem *paste_menu_item = createMenuItem<PasteMenuItem>("Paste Step");
        paste_menu_item->module = module;
        paste_menu_item->index = index;
        menu->addChild(paste_menu_item);  

        menu->addChild(new MenuSeparator);      

        // Shift steps right menu item
        ShiftRightMenuItem *shift_right_menu_item = createMenuItem<ShiftRightMenuItem>("Shift Steps Right");
        shift_right_menu_item->module = module;
        menu->addChild(shift_right_menu_item);

        // Shift steps right menu item
        ShiftLeftMenuItem *shift_left_menu_item = createMenuItem<ShiftLeftMenuItem>("Shift Steps Left");
        shift_left_menu_item->module = module;
        menu->addChild(shift_left_menu_item);  

        menu->addChild(new MenuSeparator);

        // Shift steps right menu item
        RandomizeStepsMenuItem *randomize_steps_menu_item = createMenuItem<RandomizeStepsMenuItem>("Randomize Steps");
        randomize_steps_menu_item->module = module;
        menu->addChild(randomize_steps_menu_item);

        // Shift steps right menu item
        ClearStepsMenuItem *clear_steps_menu_item = createMenuItem<ClearStepsMenuItem>("Clear Steps");
        clear_steps_menu_item->module = module;
        menu->addChild(clear_steps_menu_item);                  
    }
};
