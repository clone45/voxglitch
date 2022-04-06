struct GlitchSequencerWidget : VoxglitchModuleWidget
{
  GlitchSequencerWidget(GlitchSequencer* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/glitch_sequencer/glitch_sequencer_front_panel.svg")));

    PNGPanel *png_panel = new PNGPanel("res/glitch_sequencer/glitch_sequencer_baseplate.png", 132.08, 128.5);
    addChild(png_panel);

    // Add typography layer
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/glitch_sequencer/glitch_sequencer_typography.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);


    addInput(createInputCentered<VoxglitchInputPort>(Vec(32.627510,333.019562), module, GlitchSequencer::STEP_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(72.919708,333.169525), module, GlitchSequencer::RESET_INPUT));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(112.711708,333.069550), module, GlitchSequencer::LENGTH_KNOB));

    addParam(createParamCentered<VoxglitchRoundLampSwitch>(Vec(165.846024,324.900879), module, GlitchSequencer::SELECTION_BUTTONS + 0));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(Vec(193.594193,324.900879), module, GlitchSequencer::SELECTION_BUTTONS + 1));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(Vec(221.342362,324.900879), module, GlitchSequencer::SELECTION_BUTTONS + 2));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(Vec(249.090531,324.900879), module, GlitchSequencer::SELECTION_BUTTONS + 3));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(Vec(276.838700,324.900879), module, GlitchSequencer::SELECTION_BUTTONS + 4));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(Vec(304.586869,324.900879), module, GlitchSequencer::SELECTION_BUTTONS + 5));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(Vec(332.335038,324.900879), module, GlitchSequencer::SELECTION_BUTTONS + 6));
    addParam(createParamCentered<VoxglitchRoundLampSwitch>(Vec(360.083207,324.900879), module, GlitchSequencer::SELECTION_BUTTONS + 7));

    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(165.846024, 353.727386), module, GlitchSequencer::GATE_OUTPUT_1));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(193.594193, 353.727386), module, GlitchSequencer::GATE_OUTPUT_2));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(221.342362, 353.727386), module, GlitchSequencer::GATE_OUTPUT_3));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(249.090531, 353.727386), module, GlitchSequencer::GATE_OUTPUT_4));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(276.838700, 353.727386), module, GlitchSequencer::GATE_OUTPUT_5));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(304.586869, 353.727386), module, GlitchSequencer::GATE_OUTPUT_6));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(332.335038, 353.727386), module, GlitchSequencer::GATE_OUTPUT_7));
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(360.083207, 353.727386), module, GlitchSequencer::GATE_OUTPUT_8));


    CellularAutomatonDisplay *ca_display = new CellularAutomatonDisplay();
    ca_display->box.pos = Vec(19.9, 19.9);
    ca_display->module = module;
    addChild(ca_display);

    LengthReadoutDisplay *length_readout_display = new LengthReadoutDisplay();
    length_readout_display->box.pos = mm2px(Vec(30.0, 122.0));
    length_readout_display->module = module;
    addChild(length_readout_display);
  }

  void appendContextMenu(Menu *menu) override
  {
    GlitchSequencer *module = dynamic_cast<GlitchSequencer*>(this->module);
    assert(module);
  }

};
