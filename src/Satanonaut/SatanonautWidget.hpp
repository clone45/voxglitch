struct SatanonautWidget : ModuleWidget
{
    SatanonautWidget(Satanonaut *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/satanonaut/satanonaut_panel.svg"),
            asset::plugin(pluginInstance, "res/satanonaut/satanonaut_panel-dark.svg")
        );

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("effect_input"), module, Satanonaut::EFFECT_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("buffer_size_input"), module, Satanonaut::BUFFER_SIZE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("feedback_input"), module, Satanonaut::FEEDBACK_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("param_1_input"), module, Satanonaut::PARAM_1_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("param_2_input"), module, Satanonaut::PARAM_2_INPUT));

        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("effect_knob"), module, Satanonaut::EFFECT_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("buffer_size_knob"), module, Satanonaut::BUFFER_SIZE_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("feedback_knob"), module, Satanonaut::FEEDBACK_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("param1_knob"), module, Satanonaut::PARAM_1_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("param2_knob"), module, Satanonaut::PARAM_2_KNOB));

        // Inputs and outputs
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("left_input"), module, Satanonaut::AUDIO_INPUT_LEFT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("right_input"), module, Satanonaut::AUDIO_INPUT_RIGHT));

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, Satanonaut::AUDIO_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, Satanonaut::AUDIO_OUTPUT_RIGHT));
    }
};
