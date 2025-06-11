struct OneZeroWidget : ModuleWidget
{
    OneZeroWidget(OneZero *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/onezero/onezero_panel.svg"),
            asset::plugin(pluginInstance, "res/onezero/onezero_panel-dark.svg")
        );

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("clock_input"), module, OneZero::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, OneZero::RESET_INPUT));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("prev_input"), module, OneZero::PREV_SEQUENCE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("next_input"), module, OneZero::NEXT_SEQUENCE_INPUT));

        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("zero_button"), module, OneZero::ZERO_BUTTON_PARAM, OneZero::ZERO_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("prev_button"), module, OneZero::PREV_BUTTON_PARAM, OneZero::PREV_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("next_button"), module, OneZero::NEXT_BUTTON_PARAM, OneZero::NEXT_BUTTON_LIGHT));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("zero_input"), module, OneZero::ZERO_SEQUENCE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("cv_input"), module, OneZero::CV_SEQUENCE_SELECT));

        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("cv_attn_knob"), module, OneZero::CV_SEQUENCE_ATTN_KNOB));

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("output"), module, OneZero::GATE_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("eol_output"), module, OneZero::EOL_OUTPUT)); // end of sequence output

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
#ifdef USING_CARDINAL_NOT_RACK
            OneZero *module = this->module;
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
