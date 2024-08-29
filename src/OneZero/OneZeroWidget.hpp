struct OneZeroWidget : ModuleWidget
{
    OneZeroWidget(OneZero *module)
    {
        setModule(module);

        // Load and apply theme
        // theme.load("onezero");
        // applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/onezero/onezero_panel.svg"),
            asset::plugin(pluginInstance, "res/onezero/onezero_panel-dark.svg")
        ));

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 63.3706), module, OneZero::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 63.3706), module, OneZero::RESET_INPUT));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 157.3509), module, OneZero::PREV_SEQUENCE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 157.3509), module, OneZero::NEXT_SEQUENCE_INPUT));

        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(25.6417, 261.9237), module, OneZero::ZERO_BUTTON_PARAM, OneZero::ZERO_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(25.6417, 185.8046), module, OneZero::PREV_BUTTON_PARAM, OneZero::PREV_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(65.5748, 185.8046), module, OneZero::NEXT_BUTTON_PARAM, OneZero::NEXT_BUTTON_LIGHT));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 233.47), module, OneZero::ZERO_SEQUENCE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 233.47), module, OneZero::CV_SEQUENCE_SELECT));

        addParam(createParamCentered<Trimpot>(Vec(65.5748, 261.9237), module, OneZero::CV_SEQUENCE_ATTN_KNOB));

        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(65.5748, 342.437317), module, OneZero::GATE_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(25.6417, 342.437317), module, OneZero::EOL_OUTPUT)); // end of sequence output

        // Add display
        OneZeroReadoutWidget *one_zero_readout_widget = new OneZeroReadoutWidget();
        one_zero_readout_widget->box.pos = Vec(16.6063, 110.5335);
        one_zero_readout_widget->module = module;
        addChild(one_zero_readout_widget);
    }

    struct LoadFileMenuItem : MenuItem
    {
        OneZero *module;

        void onAction(const event::Action &e) override
        {
            std::string path = module->selectFileVCV();
            module->loadData(path);
            module->path = path;
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        OneZero *module = dynamic_cast<OneZero *>(this->module);
        assert(module);

        // Menu in development
        menu->addChild(new MenuEntry); // For spacing only

        LoadFileMenuItem *load_file_menu_item = createMenuItem<LoadFileMenuItem>("Load File");
        load_file_menu_item->module = module;
        menu->addChild(load_file_menu_item);

        if (module->path != "")
        {
            std::string filename = rack::system::getFilename(module->path);
            menu->addChild(createMenuLabel(filename));
        }
        else
        {
            menu->addChild(createMenuLabel("No file loaded"));
        }
    }
};
