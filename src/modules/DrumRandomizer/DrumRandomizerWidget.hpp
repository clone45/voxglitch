struct DrumRandomizerWidget : ModuleWidget
{
    DrumRandomizerWidget(DrumRandomizer *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/drum_randomizer/drum_randomizer_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/drum_randomizer/drum_randomizer_panel-dark.svg")
        );

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("clock_input"), module, DrumRandomizer::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, DrumRandomizer::RESET_INPUT));

        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("channel_knob"), module, DrumRandomizer::CHANNEL_KNOB));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("step_knob"), module, DrumRandomizer::STEP_KNOB));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("percentage_knob"), module, DrumRandomizer::PERCENTAGE_KNOB));

        addInput(createInputCentered<VoxglitchPolyPort>(panelHelper.findNamed("gate_input"), module, DrumRandomizer::GATE_INPUT));
        addOutput(createOutputCentered<VoxglitchPolyPort>(panelHelper.findNamed("gate_output"), module, DrumRandomizer::GATE_OUTPUT));

        // Add display
        DrumRandomizerReadoutWidget *drum_randomizer_channel_readout_widget = new DrumRandomizerReadoutWidget();
        drum_randomizer_channel_readout_widget->box.pos = Vec(10.0, 117.0);
        if (module)
            drum_randomizer_channel_readout_widget->value_pointer = &module->channel_display_value;
        addChild(drum_randomizer_channel_readout_widget);

        DrumRandomizerReadoutWidget *drum_randomizer_step_readout_widget = new DrumRandomizerReadoutWidget();
        drum_randomizer_step_readout_widget->box.pos = Vec(10.0, 182.0);
        if (module)
            drum_randomizer_step_readout_widget->value_pointer = &module->step_display_value;
        addChild(drum_randomizer_step_readout_widget);

        DrumRandomizerReadoutWidget *drum_randomizer_percent_readout_widget = new DrumRandomizerReadoutWidget();
        drum_randomizer_percent_readout_widget->box.pos = Vec(10.0, 247.0);
        if (module)
            drum_randomizer_percent_readout_widget->value_pointer = &module->percentage_display_value;
        addChild(drum_randomizer_percent_readout_widget);
    }
};
