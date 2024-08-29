struct GrainEngineMK2ExpanderWidget : ModuleWidget
{
    GrainEngineMK2ExpanderWidget(GrainEngineMK2Expander *module)
    {
        setModule(module);

        // Load and apply theme
        // theme.load("grain_engine_mk2_expander");
        // applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/grain_engine_mk2_expander/grain_engine_mk2_expander_panel.svg"),
            asset::plugin(pluginInstance, "res/grain_engine_mk2_expander/grain_engine_mk2_expander_panel-dark.svg")
        ));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(40.417305, 59.081532), module, GrainEngineMK2Expander::AUDIO_IN_LEFT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(50.382683, 96.631500), module, GrainEngineMK2Expander::AUDIO_IN_RIGHT));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(25.117329, 176.798842), module, GrainEngineMK2Expander::RECORD_START_INPUT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(25.117329, 207.750000), module, GrainEngineMK2Expander::RECORD_START_BUTTON_PARAM, GrainEngineMK2Expander::RECORDING_LIGHT));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(65.267303, 178.482361), module, GrainEngineMK2Expander::RECORD_STOP_INPUT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(65.250000, 207.750000), module, GrainEngineMK2Expander::RECORD_STOP_BUTTON_PARAM, GrainEngineMK2Expander::STOPPED_LIGHT));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(25.067327, 260.582733), module, GrainEngineMK2Expander::SAMPLE_SLOT_INPUT));
        addParam(createParamCentered<Trimpot>(Vec(65.332687, 261.082764), module, GrainEngineMK2Expander::SAMPLE_SLOT_KNOB_PARAM));

        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(24.917328, 342.437317), module, GrainEngineMK2Expander::PASSTHROUGH_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(65.082687, 342.437317), module, GrainEngineMK2Expander::PASSTHROUGH_RIGHT));
    }

    void appendContextMenu(Menu *menu) override
    {
        GrainEngineMK2Expander *module = dynamic_cast<GrainEngineMK2Expander *>(this->module);
        assert(module);
    }
};
