struct GlitchSequencerWidget : ModuleWidget
{
    GlitchSequencerWidget(GlitchSequencer *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/glitch_sequencer/glitch_sequencer_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/glitch_sequencer/glitch_sequencer_panel-dark.svg")
        );

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("step_input"), module, GlitchSequencer::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, GlitchSequencer::RESET_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("length_knob"), module, GlitchSequencer::LENGTH_KNOB));

        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("trigger_group_button_1"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 0, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 0));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("trigger_group_button_2"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 1, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 1));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("trigger_group_button_3"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 2, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 2));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("trigger_group_button_4"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 3, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 3));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("trigger_group_button_5"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 4, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 4));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("trigger_group_button_6"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 5, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 5));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("trigger_group_button_7"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 6, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 6));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("trigger_group_button_8"), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 7, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 7));

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output_1"), module, GlitchSequencer::GATE_OUTPUTS + 0));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output_2"), module, GlitchSequencer::GATE_OUTPUTS + 1));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output_3"), module, GlitchSequencer::GATE_OUTPUTS + 2));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output_4"), module, GlitchSequencer::GATE_OUTPUTS + 3));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output_5"), module, GlitchSequencer::GATE_OUTPUTS + 4));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output_6"), module, GlitchSequencer::GATE_OUTPUTS + 5));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output_7"), module, GlitchSequencer::GATE_OUTPUTS + 6));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output_8"), module, GlitchSequencer::GATE_OUTPUTS + 7));

        CellularAutomatonDisplay *ca_display = new CellularAutomatonDisplay();
        ca_display->box.pos = Vec(19.9, 19.9);
        ca_display->module = module;
        addChild(ca_display);
    }

    void appendContextMenu(Menu *menu) override
    {
        GlitchSequencer *module = dynamic_cast<GlitchSequencer *>(this->module);
        assert(module);
    }
};
