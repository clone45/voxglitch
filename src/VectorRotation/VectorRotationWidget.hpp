struct VectorRotationWidget : ModuleWidget {
    VectorRotationWidget(VectorRotation* module) {
        setModule(module);
        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/vector_rotation/vector_rotation_panel.svg"),
            asset::plugin(pluginInstance, "res/vector_rotation/vector_rotation_panel-dark.svg")
        );

        // Add rotation knobs
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("alpha_knob"), module, VectorRotation::ALPHA_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("beta_knob"), module, VectorRotation::BETA_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("gamma_knob"), module, VectorRotation::GAMMA_PARAM));
        
        // Add rotation CV inputs
        addInput(createInputCentered<PJ301MPort>(panelHelper.findNamed("alpha_input"), module, VectorRotation::ALPHA_INPUT));
        addInput(createInputCentered<PJ301MPort>(panelHelper.findNamed("beta_input"), module, VectorRotation::BETA_INPUT));
        addInput(createInputCentered<PJ301MPort>(panelHelper.findNamed("gamma_input"), module, VectorRotation::GAMMA_INPUT));
        
        // Add coordinate inputs (new)
        addInput(createInputCentered<PJ301MPort>(panelHelper.findNamed("x_input"), module, VectorRotation::X_INPUT));
        addInput(createInputCentered<PJ301MPort>(panelHelper.findNamed("y_input"), module, VectorRotation::Y_INPUT));
        
        // Add coordinate outputs
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("x_output"), module, VectorRotation::X_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("y_output"), module, VectorRotation::Y_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("z_output"), module, VectorRotation::Z_OUTPUT));
    }
};