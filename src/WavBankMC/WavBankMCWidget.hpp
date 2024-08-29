struct WavBankMCWidget : ModuleWidget
{
    WavBankMCWidget(WavBankMC *module)
    {
        setModule(module);

        // Load and apply theme
        // theme.load("wavbank_mc");
        // applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/wavbank_mc/wavbank_mc_panel.svg"),
            asset::plugin(pluginInstance, "res/wavbank_mc/wavbank_mc_panel-dark.svg")
        ));

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        addParam(createParamCentered<RoundHugeBlackKnob>(Vec(202.096863, 80.255463), module, WavBankMC::WAV_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(270.199890, 97.450043), module, WavBankMC::WAV_INPUT));
        addParam(createParamCentered<Trimpot>(Vec(270.299652, 60.532650), module, WavBankMC::WAV_ATTN_KNOB));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(179.650024, 160.600006), module, WavBankMC::TRIG_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(225.000000, 160.600006), module, WavBankMC::NEXT_WAV_TRIGGER_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(270.250000, 160.600006), module, WavBankMC::PREV_WAV_TRIGGER_INPUT));

        addParam(createParamCentered<squareToggle>(Vec(179.650024, 188.8), module, WavBankMC::TRIG_INPUT_BUTTON_PARAM));
        addParam(createParamCentered<squareToggle>(Vec(225.000000, 188.8), module, WavBankMC::NEXT_WAV_BUTTON_PARAM));
        addParam(createParamCentered<squareToggle>(Vec(270.250000, 188.8), module, WavBankMC::PREV_WAV_BUTTON_PARAM));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(179.650055, 240.359680), module, WavBankMC::PITCH_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(225.00006, 240.248474), module, WavBankMC::VOLUME_INPUT));

        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(270.199677, 240.283936), module, WavBankMC::POLY_WAV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(225.000000, 349.589600), module, WavBankMC::LEFT_WAV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(270.250549, 349.739563), module, WavBankMC::RIGHT_WAV_OUTPUT));

        addParam(createParamCentered<squareToggle>(Vec(171.816985, 348.506453), module, WavBankMC::LOOP_SWITCH));

        WavBankMCReadout *readout = new WavBankMCReadout();
        readout->box.pos = Vec(23.6220, 23.6220);
        readout->box.size = Vec(110, 30);
        readout->module = module;
        addChild(readout);
    }

    struct RestartOption : MenuItem
    {
        WavBankMC *module;
        void onAction(const event::Action &e) override
        {
            module->sample_change_mode = RESTART_PLAYBACK;
        }
    };

    struct ContinualOption : MenuItem
    {
        WavBankMC *module;
        void onAction(const event::Action &e) override
        {
            module->sample_change_mode = CONTINUAL_PLAYBACK;
        }
    };

    struct SampleChangeModeMenu : MenuItem
    {
        WavBankMC *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            RestartOption *restart_option = createMenuItem<RestartOption>("Restart", CHECKMARK(module->sample_change_mode == RESTART_PLAYBACK));
            restart_option->module = module;
            menu->addChild(restart_option);

            ContinualOption *continual_option = createMenuItem<ContinualOption>("Continual", CHECKMARK(module->sample_change_mode == CONTINUAL_PLAYBACK));
            continual_option->module = module;
            menu->addChild(continual_option);

            return menu;
        }
    };

    struct SmoothingMenuItem : MenuItem
    {
        WavBankMC *module;
        void onAction(const event::Action &e) override
        {
            module->smoothing = !(module->smoothing);
        }
    };

    //
    // menu structure for selecting between different trigger input behaviors
    //
    /*
    struct TriggerOption : MenuItem {
      WavBankMC *module;

      void onAction(const event::Action &e) override {
        module->trig_input_response_mode = TRIGGER;
      }
    };

    struct GateOption : MenuItem {
      WavBankMC *module;

      void onAction(const event::Action &e) override {
      module->trig_input_response_mode = GATE;
      }
    };

    struct TriggerModeMenu : MenuItem {
      WavBankMC *module;

      Menu *createChildMenu() override {
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
    */
    // }} End of trigger mode menu code

    void appendContextMenu(Menu *menu) override
    {
        WavBankMC *module = dynamic_cast<WavBankMC *>(this->module);
        assert(module);

        // For spacing only
        menu->addChild(new MenuEntry);

        /*
        TriggerModeMenu *trigger_mode_menu = createMenuItem<TriggerModeMenu>("Trigger Mode", RIGHT_ARROW);
        trigger_mode_menu->module = module;
        menu->addChild(trigger_mode_menu);
        */

        SampleChangeModeMenu *sample_change_mode_menu = createMenuItem<SampleChangeModeMenu>("Sample Change Behavior", RIGHT_ARROW);
        sample_change_mode_menu->module = module;
        menu->addChild(sample_change_mode_menu);

        SmoothingMenuItem *smoothing_menu_item = createMenuItem<SmoothingMenuItem>("Smoothing");
        smoothing_menu_item->rightText = CHECKMARK(module->smoothing == 1);
        smoothing_menu_item->module = module;
        menu->addChild(smoothing_menu_item);

        // Add the "Select Directory Containing WAV Files" menu item
        MenuItemLoadBankMC *menu_item_load_bank_mc = new MenuItemLoadBankMC();
        menu_item_load_bank_mc->text = "Select Directory Containing WAV Files";
        menu_item_load_bank_mc->module = module;
        menu->addChild(menu_item_load_bank_mc);
    }
};
