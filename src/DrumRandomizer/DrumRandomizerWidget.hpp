struct DrumRandomizerWidget : VoxglitchModuleWidget
{
    DrumRandomizerWidget(DrumRandomizer *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("drum_randomizer");
        applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/drum_randomizer/drum_randomizer_panel.svg"),
            asset::plugin(pluginInstance, "res/drum_randomizer/drum_randomizer_panel-dark.svg")
        ));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(33.63673, 63.3706), module, DrumRandomizer::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(86.40626, 63.3706), module, DrumRandomizer::RESET_INPUT));

        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(88.0, 132.77953), module, DrumRandomizer::CHANNEL_KNOB));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(88.0, 197.74016), module, DrumRandomizer::STEP_KNOB));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(88.0, 262.70079), module, DrumRandomizer::PERCENTAGE_KNOB));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(33.63673, 349.837158), module, DrumRandomizer::GATE_INPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(86.40626, 349.837158), module, DrumRandomizer::GATE_OUTPUT));

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
