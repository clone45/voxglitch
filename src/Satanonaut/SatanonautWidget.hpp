struct SatanonautWidget : ModuleWidget
{
    SatanonautWidget(Satanonaut *module)
    {
        setModule(module);

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/satanonaut/satanonaut_panel.svg"),
            asset::plugin(pluginInstance, "res/satanonaut/satanonaut_panel-dark.svg")));

        float space_between_inputs = 50.8;

        addInput(createInputCentered<PJ301MPort>(Vec(25.6417, 50.8), module, Satanonaut::EFFECT_INPUT));
        addInput(createInputCentered<PJ301MPort>(Vec(25.6417, 50.8 + (space_between_inputs * 1)), module, Satanonaut::BUFFER_SIZE_INPUT));
        addInput(createInputCentered<PJ301MPort>(Vec(25.6417, 50.8 + (space_between_inputs * 2)), module, Satanonaut::FEEDBACK_INPUT));
        addInput(createInputCentered<PJ301MPort>(Vec(25.6417, 50.8 + (space_between_inputs * 3)), module, Satanonaut::PARAM_1_INPUT));
        addInput(createInputCentered<PJ301MPort>(Vec(25.6417, 50.8 + (space_between_inputs * 4)), module, Satanonaut::PARAM_2_INPUT));

        addParam(createParamCentered<Trimpot>(Vec(64, 50.8), module, Satanonaut::EFFECT_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(64, 50.8 + (space_between_inputs * 1)), module, Satanonaut::BUFFER_SIZE_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(64, 50.8 + (space_between_inputs * 2)), module, Satanonaut::FEEDBACK_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(64, 50.8 + (space_between_inputs * 3)), module, Satanonaut::PARAM_1_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(64, 50.8 + (space_between_inputs * 4)), module, Satanonaut::PARAM_2_KNOB));

        // Inputs and outputs
        addInput(createInputCentered<PJ301MPort>(Vec(25.6417, 292 + 30), module, Satanonaut::AUDIO_INPUT_LEFT));
        addInput(createInputCentered<PJ301MPort>(Vec(25.6417, 326 + 30), module, Satanonaut::AUDIO_INPUT_RIGHT));

        addOutput(createOutputCentered<PJ301MPort>(Vec(64, 292 + 30), module, Satanonaut::AUDIO_OUTPUT_LEFT));
        addOutput(createOutputCentered<PJ301MPort>(Vec(64, 325 + 30), module, Satanonaut::AUDIO_OUTPUT_RIGHT));
    }
};
