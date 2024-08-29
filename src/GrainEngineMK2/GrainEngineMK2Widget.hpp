struct GrainEngineMK2Widget : VoxglitchModuleWidget
{
    GrainEngineMK2Widget(GrainEngineMK2 *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("grain_engine_mk2");
        applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/grain_engine_mk2/grain_engine_mk2_panel.svg"),
            asset::plugin(pluginInstance, "res/grain_engine_mk2/grain_engine_mk2_panel-dark.svg")
        ));

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Control inputs in the center area of the module
        addParam(createParamCentered<RoundHugeBlackKnob>(Vec(150.407593, 78.039162), module, GrainEngineMK2::POSITION_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(224.822815, 59.589138), module, GrainEngineMK2::POSITION_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(231.972626, 96.289116), module, GrainEngineMK2::POSITION_INPUT));

        addParam(createParamCentered<Trimpot>(Vec(75.375320, 59.539215), module, GrainEngineMK2::JITTER_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(67.875175, 96.400185), module, GrainEngineMK2::JITTER_INPUT));

        // Spawn rate
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(33.867897, 178.221176), module, GrainEngineMK2::RATE_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(33.867897, 223.223434), module, GrainEngineMK2::RATE_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(33.867897, 260.573914), module, GrainEngineMK2::RATE_INPUT));

        // Window
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(91.929710, 178.221176), module, GrainEngineMK2::WINDOW_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(91.929710, 223.223434), module, GrainEngineMK2::WINDOW_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(91.929710, 260.573914), module, GrainEngineMK2::WINDOW_INPUT));

        // Grains
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(149.991523, 178.221176), module, GrainEngineMK2::GRAINS_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(149.991523, 223.223434), module, GrainEngineMK2::GRAINS_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(149.991523, 260.573914), module, GrainEngineMK2::GRAINS_INPUT));

        // Pitch
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(208.303345, 178.221176), module, GrainEngineMK2::PITCH_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(208.303345, 260.573914), module, GrainEngineMK2::PITCH_INPUT));

        // Sample
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(266.415070, 178.221176), module, GrainEngineMK2::SAMPLE_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(266.415070, 223.223434), module, GrainEngineMK2::SAMPLE_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(266.415070, 260.573914), module, GrainEngineMK2::SAMPLE_INPUT));

        // Spawn trigger input
        addInput(createInputCentered<VoxglitchInputPort>(Vec(33.607643, 342.537292), module, GrainEngineMK2::SPAWN_TRIGGER_INPUT));

        // Pan input
        addInput(createInputCentered<VoxglitchInputPort>(Vec(91.887779, 342.637268), module, GrainEngineMK2::PAN_INPUT));

        // Trim knob
        addParam(createParamCentered<Trimpot>(Vec(164.087967, 342.287354), module, GrainEngineMK2::TRIM_KNOB));

        // Audio Output
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(225.577179, 342.537292), module, GrainEngineMK2::AUDIO_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(266.140045, 342.452545), module, GrainEngineMK2::AUDIO_OUTPUT_RIGHT));

        GrainEngineMK2PosDisplay *pos_display = new GrainEngineMK2PosDisplay();
        pos_display->box.pos = Vec(26.574804, 124.0157);
        pos_display->module = module;
        addChild(pos_display);
    }

    void appendContextMenu(Menu *menu) override
    {
        GrainEngineMK2 *module = dynamic_cast<GrainEngineMK2 *>(this->module);
        assert(module);

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Samples"));

        //
        // Add the sample slots to the right-click context menu
        //

        for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
        {
            GrainEngineMK2LoadSample *menu_item_load_sample = new GrainEngineMK2LoadSample();
            menu_item_load_sample->sample_number = i;
            menu_item_load_sample->text = std::to_string(i + 1) + ": " + module->loaded_filenames[i];
            menu_item_load_sample->module = module;
            menu->addChild(menu_item_load_sample);
        }
    }
};
