struct DrumRandomizerWidget : VoxglitchModuleWidget
{
  DrumRandomizerWidget(DrumRandomizer *module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("drum_randomizer");
    applyTheme();

    addInput(createInputCentered<VoxglitchInputPort>(themePos("STEP_INPUT"), module, DrumRandomizer::STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, DrumRandomizer::RESET_INPUT));

    addParam(createParamCentered<VoxglitchMediumBlackKnob>(themePos("CHANNEL_KNOB"), module, DrumRandomizer::CHANNEL_KNOB));
    addParam(createParamCentered<VoxglitchMediumBlackKnob>(themePos("STEP_KNOB"), module, DrumRandomizer::STEP_KNOB));
    addParam(createParamCentered<VoxglitchMediumBlackKnob>(themePos("PERCENTAGE_KNOB"), module, DrumRandomizer::PERCENTAGE_KNOB));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("GATE_INPUT"), module, DrumRandomizer::GATE_INPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUT"), module, DrumRandomizer::GATE_OUTPUT));

    // Add display
    DrumRandomizerReadoutWidget *drum_randomizer_channel_readout_widget = new DrumRandomizerReadoutWidget();
    drum_randomizer_channel_readout_widget->box.pos = themePos("CHANNEL_READOUT");
    if(module) drum_randomizer_channel_readout_widget->value_pointer = &module->channel_display_value;
    addChild(drum_randomizer_channel_readout_widget);

    DrumRandomizerReadoutWidget *drum_randomizer_step_readout_widget = new DrumRandomizerReadoutWidget();
    drum_randomizer_step_readout_widget->box.pos = themePos("STEP_READOUT");
    if(module) drum_randomizer_step_readout_widget->value_pointer = &module->step_display_value;
    addChild(drum_randomizer_step_readout_widget);

    DrumRandomizerReadoutWidget *drum_randomizer_percent_readout_widget = new DrumRandomizerReadoutWidget();
    drum_randomizer_percent_readout_widget->box.pos = themePos("PERCENTAGE_READOUT");
    if(module) drum_randomizer_percent_readout_widget->value_pointer = &module->percentage_display_value;
    addChild(drum_randomizer_percent_readout_widget);

  }
};
