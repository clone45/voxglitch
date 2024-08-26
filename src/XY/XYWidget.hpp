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
        addInput(createInputCentered<VoxglitchInputPort>(themePos("CLK_INPUT"), module, XY::CLK_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, XY::RESET_INPUT));

        // X,Y outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("X_OUTPUT"), module, XY::X_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("Y_OUTPUT"), module, XY::Y_OUTPUT));

        // Retrigger and punch switches
        addParam(createParamCentered<squareToggle>(themePos("RETRIGGER_SWITCH"), module, XY::RETRIGGER_SWITCH));
        addParam(createParamCentered<squareToggle>(themePos("PUNCH_SWITCH"), module, XY::PUNCH_SWITCH));

        // xy mouse entry box
        XYDisplay *xy_display;
        xy_display = new XYDisplay(module);
        xy_display->box.pos = themePos("DISPLAY");
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
