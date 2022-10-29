struct AudioPoopWidget : VoxglitchSamplerModuleWidget
{
  AudioPoopWidget(AudioPoop* module)
  {
    setModule(module);

    // Load and apply theme
    theme.load("audio_poop");
    applyTheme();

    addInput(createInputCentered<VoxglitchInputPort>(themePos("AUDIO_IN_LEFT"), module, AudioPoop::AUDIO_IN_LEFT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("AUDIO_IN_RIGHT"), module, AudioPoop::AUDIO_IN_RIGHT));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("RECORD_START_INPUT"), module, AudioPoop::RECORD_START_INPUT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("RECORD_START_BUTTON_PARAM"), module, AudioPoop::RECORD_START_BUTTON_PARAM));

    addInput(createInputCentered<VoxglitchInputPort>(themePos("RECORD_STOP_INPUT"), module, AudioPoop::RECORD_STOP_INPUT));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("RECORD_STOP_BUTTON_PARAM"), module, AudioPoop::RECORD_STOP_BUTTON_PARAM));
    /*
    addInput(createInputCentered<VoxglitchInputPort>(themePos("SAMPLE_SLOT_INPUT"), module, GrainEngineMK2Expander::SAMPLE_SLOT_INPUT));
    addParam(createParamCentered<VoxglitchAttenuator>(themePos("SAMPLE_SLOT_KNOB_PARAM"), module, GrainEngineMK2Expander::SAMPLE_SLOT_KNOB_PARAM));
    */
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("PASSTHROUGH_LEFT"), module, AudioPoop::PASSTHROUGH_LEFT));
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("PASSTHROUGH_RIGHT"), module, AudioPoop::PASSTHROUGH_RIGHT));
  }

  void appendContextMenu(Menu *menu) override
  {
    AudioPoop *module = dynamic_cast<AudioPoop*>(this->module);
    assert(module);
  }
};