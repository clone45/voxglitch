struct OnePointWidget : ModuleWidget
{
    OnePointWidget(OnePoint *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/onepoint/onepoint_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/onepoint/onepoint_panel-dark.svg")
        );

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("clock_input"), module, OnePoint::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, OnePoint::RESET_INPUT));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("prev_input"), module, OnePoint::PREV_SEQUENCE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("next_input"), module, OnePoint::NEXT_SEQUENCE_INPUT));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("zero_input"), module, OnePoint::ZERO_SEQUENCE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("cv_input"), module, OnePoint::CV_SEQUENCE_SELECT));
        
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("zero_button"), module, OnePoint::ZERO_BUTTON_PARAM, OnePoint::ZERO_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("prev_button"), module, OnePoint::PREV_BUTTON_PARAM, OnePoint::PREV_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("next_button"), module, OnePoint::NEXT_BUTTON_PARAM, OnePoint::NEXT_BUTTON_LIGHT));

        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("cv_attn_knob"), module, OnePoint::CV_SEQUENCE_ATTN_KNOB));

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("output"), module, OnePoint::CV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("eol_output"), module, OnePoint::EOL_OUTPUT)); // end of sequence output

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
#ifdef USING_CARDINAL_NOT_RACK
            OnePoint *module = this->module;
            async_dialog_filebrowser(false, NULL, NULL, "Open txt", [module](char* path) {
                if (path != NULL) {
                    module->loadData(path);
                    module->path = path;
                }
                free(path);
            });
#else
            std::string path = module->selectFileVCV();
            module->loadData(path);
            module->path = path;
#endif
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
