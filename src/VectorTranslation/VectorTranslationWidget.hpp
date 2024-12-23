struct VectorTranslationWidget : ModuleWidget {
    VectorTranslationWidget(VectorTranslation* module) {
        setModule(module);
        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/vector_translation/vector_translation_panel.svg"),
            asset::plugin(pluginInstance, "res/vector_translation/vector_translation_panel-dark.svg")
        );

        // Add translation knobs
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("trans_x_knob"), module, VectorTranslation::TRANS_X_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("trans_y_knob"), module, VectorTranslation::TRANS_Y_PARAM));
        
        // Add translation CV inputs
        addInput(createInputCentered<PJ301MPort>(panelHelper.findNamed("trans_x_input"), module, VectorTranslation::TRANS_X_INPUT));
        addInput(createInputCentered<PJ301MPort>(panelHelper.findNamed("trans_y_input"), module, VectorTranslation::TRANS_Y_INPUT));
        
        // Add coordinate inputs
        addInput(createInputCentered<PJ301MPort>(panelHelper.findNamed("x_input"), module, VectorTranslation::X_INPUT));
        addInput(createInputCentered<PJ301MPort>(panelHelper.findNamed("y_input"), module, VectorTranslation::Y_INPUT));
        
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("blank_time"), module, VectorTranslation::BLANK_TIME_PARAM));

        // Add coordinate outputs
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("x_output"), module, VectorTranslation::X_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("y_output"), module, VectorTranslation::Y_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(panelHelper.findNamed("blank_output"), module, VectorTranslation::BLANK_OUTPUT));
    }
};