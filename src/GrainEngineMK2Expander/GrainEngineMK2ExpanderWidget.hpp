struct GrainEngineMK2ExpanderWidget : ModuleWidget
{
    GrainEngineMK2ExpanderWidget(GrainEngineMK2Expander *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/grain_engine_mk2_expander/grain_engine_mk2_expander_panel.svg"),
            asset::plugin(pluginInstance, "res/grain_engine_mk2_expander/grain_engine_mk2_expander_panel-dark.svg")
        );

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("left_input"), module, GrainEngineMK2Expander::AUDIO_IN_LEFT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("right_input"), module, GrainEngineMK2Expander::AUDIO_IN_RIGHT));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("start_input"), module, GrainEngineMK2Expander::RECORD_START_INPUT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("start_button"), module, GrainEngineMK2Expander::RECORD_START_BUTTON_PARAM, GrainEngineMK2Expander::RECORDING_LIGHT));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("stop_input"), module, GrainEngineMK2Expander::RECORD_STOP_INPUT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("stop_button"), module, GrainEngineMK2Expander::RECORD_STOP_BUTTON_PARAM, GrainEngineMK2Expander::STOPPED_LIGHT));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("sample_input"), module, GrainEngineMK2Expander::SAMPLE_SLOT_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("sample_attn_knob"), module, GrainEngineMK2Expander::SAMPLE_SLOT_KNOB_PARAM));

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, GrainEngineMK2Expander::PASSTHROUGH_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, GrainEngineMK2Expander::PASSTHROUGH_RIGHT));
    }

    void appendContextMenu(Menu *menu) override
    {
        GrainEngineMK2Expander *module = dynamic_cast<GrainEngineMK2Expander *>(this->module);
        assert(module);
    }
};
