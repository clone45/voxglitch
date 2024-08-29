struct XYWidget : VoxglitchModuleWidget
{
    XYWidget(XY *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("xy");
        applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/xy/xy_panel.svg"),
            asset::plugin(pluginInstance, "res/xy/xy_panel-dark.svg")
        ));

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Clock and Reset inputs
        addInput(createInputCentered<VoxglitchInputPort>(Vec(32.696091, 343.245789), module, XY::CLK_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(73.296082, 343.245789), module, XY::RESET_INPUT));

        // X,Y outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(232.399994, 343.099976), module, XY::X_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(270.450470, 343.099976), module, XY::Y_OUTPUT));

        // Retrigger and punch switches
        addParam(createParamCentered<squareToggle>(Vec(126.240843, 341.270369), module, XY::RETRIGGER_SWITCH));
        addParam(createParamCentered<squareToggle>(Vec(170.46704, 341.116988), module, XY::PUNCH_SWITCH));

        // xy mouse entry box
        XYDisplay *xy_display;
        xy_display = new XYDisplay(module);
        xy_display->box.pos = Vec(19.750000, 19.750000);
        addChild(xy_display);
    }

    struct ClicklessOption : MenuItem
    {
        XY *module;

        void onAction(const event::Action &e) override
        {
            module->tablet_mode ^= true; // flip the value
        }
    };

    struct OutputRangeValueItem : MenuItem
    {
        XY *module;
        int range_index = 0;

        void onAction(const event::Action &e) override
        {
            module->voltage_range_index = range_index;
        }
    };

    struct RangeOption : MenuItem
    {
        XY *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            for (unsigned int i = 0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
            {
                OutputRangeValueItem *output_range_value_menu_item = createMenuItem<OutputRangeValueItem>(module->voltage_range_names[i], CHECKMARK(module->voltage_range_index == i));
                output_range_value_menu_item->module = module;
                output_range_value_menu_item->range_index = i;
                menu->addChild(output_range_value_menu_item);
            }

            return menu;
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        XY *module = dynamic_cast<XY *>(this->module);
        assert(module);

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Options"));

        // Output range selection
        RangeOption *range_option = createMenuItem<RangeOption>("Output Range", RIGHT_ARROW);
        range_option->module = module;
        menu->addChild(range_option);

        // Tablet mode selection
        ClicklessOption *clickless_option = createMenuItem<ClicklessOption>("Tablet Mode", CHECKMARK(module->tablet_mode));
        clickless_option->module = module;
        menu->addChild(clickless_option);
    }
};
