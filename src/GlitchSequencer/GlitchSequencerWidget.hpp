struct GlitchSequencerWidget : VoxglitchModuleWidget
{
    GlitchSequencerWidget(GlitchSequencer *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("glitch_sequencer");
        applyTheme();

        // setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/glitch_sequencer/panel.svg")));

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/glitch_sequencer/glitch_sequencer_panel.svg"),
            asset::plugin(pluginInstance, "res/glitch_sequencer/glitch_sequencer_panel-dark.svg")
        ));

        addInput(createInputCentered<VoxglitchInputPort>(themePos("STEP_INPUT"), module, GlitchSequencer::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, GlitchSequencer::RESET_INPUT));
        addParam(createParamCentered<Trimpot>(themePos("LENGTH_KNOB"), module, GlitchSequencer::LENGTH_KNOB));

        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("TRIGGER_GROUP_BUTTONS_1"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 0, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 0));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("TRIGGER_GROUP_BUTTONS_2"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 1, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 1));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("TRIGGER_GROUP_BUTTONS_3"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 2, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 2));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("TRIGGER_GROUP_BUTTONS_4"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 3, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 3));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("TRIGGER_GROUP_BUTTONS_5"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 4, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 4));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("TRIGGER_GROUP_BUTTONS_6"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 5, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 5));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("TRIGGER_GROUP_BUTTONS_7"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 6, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 6));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(themePos("TRIGGER_GROUP_BUTTONS_8"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 7, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 7));

        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUTS_1"), module, GlitchSequencer::GATE_OUTPUTS + 0));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUTS_2"), module, GlitchSequencer::GATE_OUTPUTS + 1));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUTS_3"), module, GlitchSequencer::GATE_OUTPUTS + 2));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUTS_4"), module, GlitchSequencer::GATE_OUTPUTS + 3));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUTS_5"), module, GlitchSequencer::GATE_OUTPUTS + 4));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUTS_6"), module, GlitchSequencer::GATE_OUTPUTS + 5));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUTS_7"), module, GlitchSequencer::GATE_OUTPUTS + 6));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUTS_8"), module, GlitchSequencer::GATE_OUTPUTS + 7));

        CellularAutomatonDisplay *ca_display = new CellularAutomatonDisplay();
        ca_display->box.pos = themePos("GRID_DISPLAY");
        ca_display->module = module;
        addChild(ca_display);
    }

    void appendContextMenu(Menu *menu) override
    {
        GlitchSequencer *module = dynamic_cast<GlitchSequencer *>(this->module);
        assert(module);
    }
};
