struct ByteBeatWidget : ModuleWidget
{
    ByteBeatWidget(ByteBeat *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/bytebeat/bytebeat_panel.svg"),
            asset::plugin(pluginInstance, "res/bytebeat/bytebeat_panel-dark.svg")
        );

        // =================== PLACE COMPONENTS ====================================

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Equation
        auto equation_knob = createParamCentered<RoundHugeBlackKnob>(panelHelper.findNamed("equation_knob"), module, ByteBeat::EQUATION_KNOB);
        dynamic_cast<Knob *>(equation_knob)->snap = true;
        addParam(equation_knob);
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("equation_input"), module, ByteBeat::EQUATION_INPUT));

        // Pitch
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pitch_knob"), module, ByteBeat::CLOCK_DIVISION_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pitch_input"), module, ByteBeat::CLOCK_CV_INPUT));

        // ============================ PARAMETERS ================================

        // Knob 1
        auto knob_1 = createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("param_knob_1"), module, ByteBeat::PARAM_KNOB_1);
        dynamic_cast<Knob *>(knob_1)->snap = true;
        addParam(knob_1);

        // Input 1
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("param_input_1"), module, ByteBeat::PARAM_INPUT_1));

        // Knob 2
        auto knob_2 = createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("param_knob_2"), module, ByteBeat::PARAM_KNOB_2);
        dynamic_cast<Knob *>(knob_2)->snap = true;
        addParam(knob_2);

        // Input 2
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("param_input_2"), module, ByteBeat::PARAM_INPUT_2));

        // Knob 3
        auto knob_3 = createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("param_knob_3"), module, ByteBeat::PARAM_KNOB_3);
        dynamic_cast<Knob *>(knob_3)->snap = true;
        addParam(knob_3);

        // Input 3
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("param_input_3"), module, ByteBeat::PARAM_INPUT_3));

        // ============================ OUTPUTS ===================================

        // Output
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("audio_output"), module, ByteBeat::AUDIO_OUTPUT));



        // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_5, ROW_11)), module, ByteBeat::T_INPUT));
        // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(COLUMN_5, ROW_13)), module, ByteBeat::SYNC_CLOCK_INPUT));
    }

    /*
    void add_snapping_parameter_knob(float column, float row, int index)
    {
      auto P = createParamCentered<RoundBlackKnob>(mm2px(Vec(column, row)), module, index);
      dynamic_cast<Knob*>(P)->snap = true;
      addParam(P);
    }
    */
};
