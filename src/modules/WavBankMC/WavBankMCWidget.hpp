struct WavBankMCWidget : ModuleWidget
{
    WavBankMCWidget(WavBankMC *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/wavbank_mc/wavbank_mc_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/wavbank_mc/wavbank_mc_panel-dark.svg")
        );

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        // Sample selection controls
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("sample_knob"), module, WavBankMC::WAV_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("sample_input"), module, WavBankMC::WAV_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("sample_attn_knob"), module, WavBankMC::WAV_ATTN_KNOB));

        // Start position controls
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("start_knob"), module, WavBankMC::START_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("start_input"), module, WavBankMC::START_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("start_attn_knob"), module, WavBankMC::START_ATTN_KNOB));

        // End position controls
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("end_knob"), module, WavBankMC::END_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("end_input"), module, WavBankMC::END_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("end_attn_knob"), module, WavBankMC::END_ATTN_KNOB));

        // Attack envelope controls
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("attack_knob"), module, WavBankMC::ATTACK_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("attack_input"), module, WavBankMC::ATTACK_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("attack_attn_knob"), module, WavBankMC::ATTACK_ATTN_KNOB));

        // Decay envelope controls
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("decay_knob"), module, WavBankMC::DECAY_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("decay_input"), module, WavBankMC::DECAY_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("decay_attn_knob"), module, WavBankMC::DECAY_ATTN_KNOB));

        // Attack and Decay LED indicators
        addChild(createLightCentered<SmallLight<GreenLight>>(panelHelper.findNamed("attack_led"), module, WavBankMC::ATTACK_LED_LIGHT));
        addChild(createLightCentered<SmallLight<GreenLight>>(panelHelper.findNamed("decay_led"), module, WavBankMC::DECAY_LED_LIGHT));

        // Trigger and navigation controls
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input"), module, WavBankMC::TRIG_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("next_input"), module, WavBankMC::NEXT_WAV_TRIGGER_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("prev_input"), module, WavBankMC::PREV_WAV_TRIGGER_INPUT));

        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("trig_button"), module, WavBankMC::TRIG_INPUT_BUTTON_PARAM, WavBankMC::TRIG_INPUT_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("next_button"), module, WavBankMC::NEXT_WAV_BUTTON_PARAM, WavBankMC::NEXT_WAV_BUTTON_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("prev_button"), module, WavBankMC::PREV_WAV_BUTTON_PARAM, WavBankMC::PREV_WAV_BUTTON_LIGHT));

        // Pitch and volume CV inputs with attenuverters
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pitch_input"), module, WavBankMC::PITCH_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("pitch_attn_knob"), module, WavBankMC::PITCH_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("volume_input"), module, WavBankMC::VOLUME_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("vol_attn_knob"), module, WavBankMC::VOL_ATTN_KNOB));

        // Audio outputs
        addOutput(createOutputCentered<VoxglitchPolyPort>(panelHelper.findNamed("poly_audio_output"), module, WavBankMC::POLY_WAV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_audio_output"), module, WavBankMC::LEFT_WAV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_audio_output"), module, WavBankMC::RIGHT_WAV_OUTPUT));

        // EOC output and division knob
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("eoc_output"), module, WavBankMC::EOC_OUTPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("division_output"), module, WavBankMC::DIVISION_KNOB));

        // Loop switch and CV input
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("loop_switch"), module, WavBankMC::LOOP_SWITCH));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("loop_input"), module, WavBankMC::LOOP_INPUT));

        WavBankMCReadout *readout = new WavBankMCReadout();
        readout->box.pos = Vec(23.6220, 33.6220);
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

    // Envelope mode menu items
    struct EnvelopeADOption : MenuItem
    {
        WavBankMC *module;
        void onAction(const event::Action &e) override
        {
            module->envelope_mode = WavBankMC::ENVELOPE_AD;
        }
    };

    struct EnvelopeStartEndOption : MenuItem
    {
        WavBankMC *module;
        void onAction(const event::Action &e) override
        {
            module->envelope_mode = WavBankMC::ENVELOPE_START_END;
        }
    };

    struct EnvelopeDisabledOption : MenuItem
    {
        WavBankMC *module;
        void onAction(const event::Action &e) override
        {
            module->envelope_mode = WavBankMC::ENVELOPE_DISABLED;
        }
    };

    struct EnvelopeModeMenu : MenuItem
    {
        WavBankMC *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            EnvelopeADOption *ad_option = createMenuItem<EnvelopeADOption>("AD Envelope (decay after attack)", CHECKMARK(module->envelope_mode == WavBankMC::ENVELOPE_AD));
            ad_option->module = module;
            menu->addChild(ad_option);

            EnvelopeStartEndOption *start_end_option = createMenuItem<EnvelopeStartEndOption>("Start/End (attack at start, decay at end)", CHECKMARK(module->envelope_mode == WavBankMC::ENVELOPE_START_END));
            start_end_option->module = module;
            menu->addChild(start_end_option);

            EnvelopeDisabledOption *disabled_option = createMenuItem<EnvelopeDisabledOption>("Disabled", CHECKMARK(module->envelope_mode == WavBankMC::ENVELOPE_DISABLED));
            disabled_option->module = module;
            menu->addChild(disabled_option);

            return menu;
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

        EnvelopeModeMenu *envelope_mode_menu = createMenuItem<EnvelopeModeMenu>("Envelope Mode", RIGHT_ARROW);
        envelope_mode_menu->module = module;
        menu->addChild(envelope_mode_menu);

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
