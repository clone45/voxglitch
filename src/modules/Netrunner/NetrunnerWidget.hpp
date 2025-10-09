struct NetrunnerWidget : VoxglitchSamplerModuleWidget
{
    NetrunnerWidget(Netrunner *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/netrunner/netrunner_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/netrunner/netrunner_panel-dark.svg")
        );

        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundHugeBlackKnob>(panelHelper.findNamed("wav_knob"), module, Netrunner::WAV_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("wav_attn_knob"), module, Netrunner::WAV_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("wav_input"), module, Netrunner::WAV_INPUT));

        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("loop_switch"), module, Netrunner::LOOP_SWITCH));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input"), module, Netrunner::TRIG_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pitch_input"), module, Netrunner::PITCH_INPUT));

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, Netrunner::WAV_LEFT_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, Netrunner::WAV_RIGHT_OUTPUT));

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("end_of_sample_output"), module, Netrunner::END_OF_SAMPLE_OUTPUT));

        NetrunnerReadout *readout = new NetrunnerReadout();
        readout->box.pos = Vec(13.300354, 141.0);
        readout->box.size = Vec(220, 30);
        readout->module = module;
        addChild(readout);
    }

    struct TriggerOption : MenuItem
    {
        Netrunner *module;

        void onAction(const event::Action &e) override
        {
            module->trig_input_response_mode = TRIGGER;
        }
    };

    struct GateOption : MenuItem
    {
        Netrunner *module;

        void onAction(const event::Action &e) override
        {
            module->trig_input_response_mode = GATE;
        }
    };

    struct TriggerModeMenu : MenuItem
    {
        Netrunner *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            TriggerOption *trigger_option = createMenuItem<TriggerOption>("Trigger", CHECKMARK(module->trig_input_response_mode == TRIGGER));
            trigger_option->module = module;
            menu->addChild(trigger_option);

            GateOption *gate_option = createMenuItem<GateOption>("Gate", CHECKMARK(module->trig_input_response_mode == GATE));
            gate_option->module = module;
            menu->addChild(gate_option);

            return menu;
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        Netrunner *module = dynamic_cast<Netrunner *>(this->module);
        assert(module);

        menu->addChild(new MenuEntry);

        MenuItemLoadConfig *menu_item_load_config = new MenuItemLoadConfig();
        menu_item_load_config->text = "Load Config File (JSON)";
        menu_item_load_config->module = module;
        menu->addChild(menu_item_load_config);

        menu->addChild(new MenuEntry);

        TriggerModeMenu *trigger_mode_menu = createMenuItem<TriggerModeMenu>("Trigger Mode", RIGHT_ARROW);
        trigger_mode_menu->module = module;
        menu->addChild(trigger_mode_menu);

        menu->addChild(createBoolPtrMenuItem("Auto-Increment on Completion", "", &module->auto_increment_on_completion));
        menu->addChild(createBoolPtrMenuItem("Send Rack Token", "", &module->send_rack_token));

        menu->addChild(new MenuEntry);
        SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
        sample_interpolation_menu_item->module = module;
        menu->addChild(sample_interpolation_menu_item);
    }
};