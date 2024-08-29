struct GlitchSequencerWidget : ModuleWidget
{
    GlitchSequencerWidget(GlitchSequencer *module)
    {
        setModule(module);

        // Load and apply theme
        // theme.load("glitch_sequencer");
        // applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/glitch_sequencer/glitch_sequencer_panel.svg"),
            asset::plugin(pluginInstance, "res/glitch_sequencer/glitch_sequencer_panel-dark.svg")
        ));

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(32.627510, 333.019562), module, GlitchSequencer::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(72.919708, 333.169525), module, GlitchSequencer::RESET_INPUT));
        addParam(createParamCentered<Trimpot>(Vec(112.711708, 333.069550), module, GlitchSequencer::LENGTH_KNOB));

        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(165.846024, 323.217879), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 0, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 0));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(193.594193, 323.217879), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 1, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 1));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(221.342362, 323.217879), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 2, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 2));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(249.090531, 323.217879), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 3, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 3));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(276.838700, 323.217879), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 4, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 4));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(304.586869, 323.217879), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 5, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 5));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(332.335038, 323.217879), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 6, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 6));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(360.083207, 323.217879), module, GlitchSequencer::TRIGGER_GROUP_BUTTONS + 7, GlitchSequencer::TRIGGER_GROUP_LIGHTS + 7));

        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(165.846024, 353.727386), module, GlitchSequencer::GATE_OUTPUTS + 0));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(193.594193, 353.727386), module, GlitchSequencer::GATE_OUTPUTS + 1));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(221.342362, 353.727386), module, GlitchSequencer::GATE_OUTPUTS + 2));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(249.090531, 353.727386), module, GlitchSequencer::GATE_OUTPUTS + 3));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(276.838700, 353.727386), module, GlitchSequencer::GATE_OUTPUTS + 4));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(304.586869, 353.727386), module, GlitchSequencer::GATE_OUTPUTS + 5));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(332.335038, 353.727386), module, GlitchSequencer::GATE_OUTPUTS + 6));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(360.083207, 353.727386), module, GlitchSequencer::GATE_OUTPUTS + 7));

        CellularAutomatonDisplay *ca_display = new CellularAutomatonDisplay();
        ca_display->box.pos = Vec(19.9, 19.9);
        ca_display->module = module;
        addChild(ca_display);
    }

    void appendContextMenu(Menu *menu) override
    {
        GlitchSequencer *module = dynamic_cast<GlitchSequencer *>(this->module);
        assert(module);
    }
};
