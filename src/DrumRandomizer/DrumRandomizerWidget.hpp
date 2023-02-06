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

    addParam(createParamCentered<VoxglitchAttenuator>(themePos("CHANNEL_KNOB"), module, DrumRandomizer::CHANNEL_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("STEP_KNOB"), module, DrumRandomizer::STEP_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("PERCENTAGE_KNOB"), module, DrumRandomizer::PERCENTAGE_KNOB));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("GATE_INPUT"), module, DrumRandomizer::GATE_INPUT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("GATE_OUTPUT"), module, DrumRandomizer::GATE_OUTPUT));

  }
};
