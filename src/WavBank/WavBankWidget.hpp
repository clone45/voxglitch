struct WavBankWidget : VoxglitchSamplerModuleWidget
{
    WavBankWidget(WavBank *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("wavbank");
        applyTheme();

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/wavbank/panel.svg")));

        addParam(createParamCentered<VoxglitchLargeKnob>(themePos("WAV_KNOB"), module, WavBank::WAV_KNOB));
        addParam(createParamCentered<VoxglitchAttenuator>(themePos("WAV_ATTN_KNOB"), module, WavBank::WAV_ATTN_KNOB));
        addParam(createParamCentered<squareToggle>(themePos("LOOP_SWITCH"), module, WavBank::LOOP_SWITCH));

        // Input jacks
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIG_INPUT"), module, WavBank::TRIG_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("WAV_INPUT"), module, WavBank::WAV_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("PITCH_INPUT"), module, WavBank::PITCH_INPUT));

        // WAV output
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("WAV_LEFT_OUTPUT"), module, WavBank::WAV_LEFT_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("WAV_RIGHT_OUTPUT"), module, WavBank::WAV_RIGHT_OUTPUT));

        WavBankReadout *readout = new WavBankReadout();
        readout->box.pos = themePos("READOUT");
        readout->box.size = Vec(110, 30); // bounding box of the widget
        readout->module = module;
        addChild(readout);
    }

    //
    // menu structure for selecting between different trigger input behaviors
    //

    struct TriggerOption : MenuItem
    {
        WavBank *module;

        void onAction(const event::Action &e) override
        {
            module->trig_input_response_mode = TRIGGER;
        }
    };

    struct GateOption : MenuItem
    {
        WavBank *module;

        void onAction(const event::Action &e) override
        {
            module->trig_input_response_mode = GATE;
        }
    };

    struct TriggerModeMenu : MenuItem
    {
        WavBank *module;

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

    // }} End of trigger mode menu code

    void appendContextMenu(Menu *menu) override
    {
        WavBank *module = dynamic_cast<WavBank *>(this->module);
        assert(module);

        // For spacing only
        menu->addChild(new MenuEntry);

        TriggerModeMenu *trigger_mode_menu = createMenuItem<TriggerModeMenu>("Trigger Mode", RIGHT_ARROW);
        trigger_mode_menu->module = module;
        menu->addChild(trigger_mode_menu);

        // Add the "Select Directory Containing WAV Files" menu item
        MenuItemLoadBank *menu_item_load_bank = new MenuItemLoadBank();
        menu_item_load_bank->text = "Select Directory Containing WAV Files";
        menu_item_load_bank->module = module;
        menu->addChild(menu_item_load_bank);

        // Sample interpolation settings
        menu->addChild(new MenuEntry); // For spacing only
        SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
        sample_interpolation_menu_item->module = module;
        menu->addChild(sample_interpolation_menu_item);
    }
};
