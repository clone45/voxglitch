struct ConferenceCallWidget : ModuleWidget
{
    ConferenceCallWidget(ConferenceCall *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/conference_call/conference_call_panel.svg"),
            asset::plugin(pluginInstance, "res/conference_call/conference_call_panel-dark.svg")
        );

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Inputs and outputs
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("left_input"), module, ConferenceCall::AUDIO_INPUT_LEFT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("right_input"), module, ConferenceCall::AUDIO_INPUT_RIGHT));

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, ConferenceCall::AUDIO_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, ConferenceCall::AUDIO_OUTPUT_RIGHT));

        // Debug dump trigger
        // addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("dump_input"), module, ConferenceCall::DUMP_INPUT));

        // PLC strategy selector
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("plc_strategy_knob"), module, ConferenceCall::PLC_STRATEGY_PARAM));

        // Compression toggle and light
        addParam(createParamCentered<CKSS>(panelHelper.findNamed("compression_toggle"), module, ConferenceCall::COMPRESSION_PARAM));
        addChild(createLightCentered<MediumLight<RedLight>>(panelHelper.findNamed("compression_light"), module, ConferenceCall::COMPRESSION_LIGHT));

        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("dropout_knob"), module, ConferenceCall::DROPOUT_PARAM));
    }
};
