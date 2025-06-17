struct GrainEngineMK2Widget : ModuleWidget
{
    GrainEngineMK2Widget(GrainEngineMK2 *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/grain_engine_mk2/grain_engine_mk2_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/grain_engine_mk2/grain_engine_mk2_panel-dark.svg")
        );

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Control inputs in the center area of the module
        addParam(createParamCentered<RoundHugeBlackKnob>(panelHelper.findNamed("position_knob"), module, GrainEngineMK2::POSITION_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("position_attn_knob"), module, GrainEngineMK2::POSITION_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("position_input"), module, GrainEngineMK2::POSITION_INPUT));

        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("jitter_knob"), module, GrainEngineMK2::JITTER_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("jitter_input"), module, GrainEngineMK2::JITTER_INPUT));

        // Spawn rate
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("rate_knob"), module, GrainEngineMK2::RATE_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("rate_attn_knob"), module, GrainEngineMK2::RATE_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("rate_input"), module, GrainEngineMK2::RATE_INPUT));

        // Window
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("window_knob"), module, GrainEngineMK2::WINDOW_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("window_attn_knob"), module, GrainEngineMK2::WINDOW_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("window_input"), module, GrainEngineMK2::WINDOW_INPUT));

        // Grains
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("grains_knob"), module, GrainEngineMK2::GRAINS_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("grains_attn_knob"), module, GrainEngineMK2::GRAINS_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("grains_input"), module, GrainEngineMK2::GRAINS_INPUT));

        // Pitch
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pitch_knob"), module, GrainEngineMK2::PITCH_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pitch_input"), module, GrainEngineMK2::PITCH_INPUT));

        // Sample
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("sample_knob"), module, GrainEngineMK2::SAMPLE_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("sample_attn_knob"), module, GrainEngineMK2::SAMPLE_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("sample_input"), module, GrainEngineMK2::SAMPLE_INPUT));

        // Spawn trigger input
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("clock_input"), module, GrainEngineMK2::SPAWN_TRIGGER_INPUT));

        // Pan input
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pan_input"), module, GrainEngineMK2::PAN_INPUT));

        // Trim knob
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("trim_knob"), module, GrainEngineMK2::TRIM_KNOB));

        // Audio Output
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, GrainEngineMK2::AUDIO_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, GrainEngineMK2::AUDIO_OUTPUT_RIGHT));

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
