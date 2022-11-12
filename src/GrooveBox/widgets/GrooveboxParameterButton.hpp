struct GrooveboxParameterButton : GrooveboxSoftButton
{
    unsigned int parameter_index = 0;

    /* Maybe add this later, although it will take significant work
    struct RandomizeParamMenuItem : MenuItem
    {
        GrooveBox *module;
        unsigned int parameter_index = 0;

        void onAction(const event::Action &e) override
        {
            module->randomizParameter(parameter_index);
        }
    };

    struct ResetParamMenuItem : MenuItem
    {
        GrooveBox *module;
        unsigned int parameter_index = 0;

        void onAction(const event::Action &e) override
        {
            module->resetParameter(parameter_index);
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        GrooveBox *module = dynamic_cast<GrooveBox *>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator);

        // Randomize parameters
        RandomizeParamMenuItem *randomize_param_menu_item = createMenuItem<RandomizeParamMenuItem>("Randomize " + FUNCTION_NAMES[this->parameter_index]);
        randomize_param_menu_item->module = module;
        randomize_param_menu_item->parameter_index = this->parameter_index;
        menu->addChild(randomize_param_menu_item);

        // Reset all knobs
        ResetParamMenuItem *reset_param_menu_item = createMenuItem<ResetParamMenuItem>("Reset Knobs");
        reset_param_menu_item->module = module;
        reset_param_menu_item->parameter_index = this->parameter_index;
        menu->addChild(reset_param_menu_item);
    }
    */
};
