struct OnePointWidget : ModuleWidget
{
    OnePointWidget(OnePoint *module)
    {
        setModule(module);

        // Load and apply theme
        // theme.load("onepoint");
        // applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/onepoint/onepoint_panel.svg"),
            asset::plugin(pluginInstance, "res/onepoint/onepoint_panel-dark.svg")
        ));

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 63.3706), module, OnePoint::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 63.3706), module, OnePoint::RESET_INPUT));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 157.3509), module, OnePoint::PREV_SEQUENCE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 157.3509), module, OnePoint::NEXT_SEQUENCE_INPUT));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(25.6417, 233.47), module, OnePoint::ZERO_SEQUENCE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(65.5748, 233.47), module, OnePoint::CV_SEQUENCE_SELECT));
        
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(25.6417, 261.9237), module, OnePoint::ZERO_BUTTON_PARAM, OnePoint::ZERO_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(25.6417, 185.8046), module, OnePoint::PREV_BUTTON_PARAM, OnePoint::PREV_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(65.5748, 185.8046), module, OnePoint::NEXT_BUTTON_PARAM, OnePoint::NEXT_BUTTON_LIGHT));

        addParam(createParamCentered<Trimpot>(Vec(65.5748, 261.9237), module, OnePoint::CV_SEQUENCE_ATTN_KNOB));

        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(65.5748, 342.437317), module, OnePoint::CV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(25.6417, 342.437317), module, OnePoint::EOL_OUTPUT)); // end of sequence output

        // Add display
        OnePointReadoutWidget *one_point_readout_widget = new OnePointReadoutWidget();
        one_point_readout_widget->box.pos = Vec(16.6063, 110.5335);
        one_point_readout_widget->module = module;
        addChild(one_point_readout_widget);
    }

    struct LoadFileMenuItem : MenuItem
    {
        OnePoint *module;

        void onAction(const event::Action &e) override
        {
            std::string path = module->selectFileVCV();
            module->loadData(path);
            module->path = path;
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        OnePoint *module = dynamic_cast<OnePoint *>(this->module);
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
