struct WavBankMCWidget : ModuleWidget
{
    WavBankMCWidget(WavBankMC *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/wavbank_mc/wavbank_mc_panel.svg"),
            asset::plugin(pluginInstance, "res/wavbank_mc/wavbank_mc_panel-dark.svg")
        );

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        addParam(createParamCentered<RoundHugeBlackKnob>(panelHelper.findNamed("wav_knob"), module, WavBankMC::WAV_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("wav_input"), module, WavBankMC::WAV_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("wav_attn_knob"), module, WavBankMC::WAV_ATTN_KNOB));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input"), module, WavBankMC::TRIG_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("next_input"), module, WavBankMC::NEXT_WAV_TRIGGER_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("prev_input"), module, WavBankMC::PREV_WAV_TRIGGER_INPUT));

        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("trig_button"), module, WavBankMC::TRIG_INPUT_BUTTON_PARAM));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("next_button"), module, WavBankMC::NEXT_WAV_BUTTON_PARAM));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("prev_button"), module, WavBankMC::PREV_WAV_BUTTON_PARAM));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pitch_input"), module, WavBankMC::PITCH_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("volume_input"), module, WavBankMC::VOLUME_INPUT));

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("poly_wav_output"), module, WavBankMC::POLY_WAV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_wav_output"), module, WavBankMC::LEFT_WAV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_wav_output"), module, WavBankMC::RIGHT_WAV_OUTPUT));

        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("loop_switch"), module, WavBankMC::LOOP_SWITCH));

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
