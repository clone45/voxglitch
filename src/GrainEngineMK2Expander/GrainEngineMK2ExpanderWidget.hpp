struct GrainEngineMK2ExpanderWidget : VoxglitchModuleWidget
{
  GrainEngineMK2ExpanderWidget(GrainEngineMK2Expander* module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("grain_engine_mk2_expander");
    applyTheme();

    addInput(createInputCentered<VoxglitchInputPort>(themePos("AUDIO_IN_LEFT"), module, GrainEngineMK2Expander::AUDIO_IN_LEFT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("AUDIO_IN_RIGHT"), module, GrainEngineMK2Expander::AUDIO_IN_RIGHT));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("RECORD_START_INPUT"), module, GrainEngineMK2Expander::RECORD_START_INPUT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("RECORD_START_BUTTON_PARAM"), module, GrainEngineMK2Expander::RECORD_START_BUTTON_PARAM));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("RECORD_STOP_INPUT"), module, GrainEngineMK2Expander::RECORD_STOP_INPUT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("RECORD_STOP_BUTTON_PARAM"), module, GrainEngineMK2Expander::RECORD_STOP_BUTTON_PARAM));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("SAMPLE_SLOT_INPUT"), module, GrainEngineMK2Expander::SAMPLE_SLOT_INPUT));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("SAMPLE_SLOT_KNOB_PARAM"), module, GrainEngineMK2Expander::SAMPLE_SLOT_KNOB_PARAM));

    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("PASSTHROUGH_LEFT"), module, GrainEngineMK2Expander::PASSTHROUGH_LEFT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("PASSTHROUGH_RIGHT"), module, GrainEngineMK2Expander::PASSTHROUGH_RIGHT));
  }

  void appendContextMenu(Menu *menu) override
  {
    GrainEngineMK2Expander *module = dynamic_cast<GrainEngineMK2Expander*>(this->module);
    assert(module);
  }
};
