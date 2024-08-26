struct GrainEngineMK2Widget : VoxglitchModuleWidget
{
    GrainEngineMK2Widget(GrainEngineMK2 *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("grain_engine_mk2");
        applyTheme();

        // setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/grain_engine_mk2/panel.svg")));

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/grain_engine_mk2/grain_engine_mk2_panel.svg"),
            asset::plugin(pluginInstance, "res/grain_engine_mk2/grain_engine_mk2_panel-dark.svg")
        ));

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        //
        // Control inputs in the center area of the module
        //
        addParam(createParamCentered<RoundHugeBlackKnob>(themePos("POSITION_KNOB"), module, GrainEngineMK2::POSITION_KNOB));
        addParam(createParamCentered<Trimpot>(themePos("POSITION_ATTN_KNOB"), module, GrainEngineMK2::POSITION_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUT"), module, GrainEngineMK2::POSITION_INPUT));

        addParam(createParamCentered<Trimpot>(themePos("JITTER_KNOB"), module, GrainEngineMK2::JITTER_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("JITTER_INPUT"), module, GrainEngineMK2::JITTER_INPUT));

        // Spawn rate
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("RATE_KNOB"), module, GrainEngineMK2::RATE_KNOB));
        addParam(createParamCentered<Trimpot>(themePos("RATE_ATTN_KNOB"), module, GrainEngineMK2::RATE_ATTN_KNOB));
        
        addInput(createInputCentered<VoxglitchInputPort>(themePos("RATE_INPUT"), module, GrainEngineMK2::RATE_INPUT));

        // Window
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("WINDOW_KNOB"), module, GrainEngineMK2::WINDOW_KNOB));
        addParam(createParamCentered<Trimpot>(themePos("WINDOW_ATTN_KNOB"), module, GrainEngineMK2::WINDOW_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("WINDOW_INPUT"), module, GrainEngineMK2::WINDOW_INPUT));

        // Grains
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("GRAINS_KNOB"), module, GrainEngineMK2::GRAINS_KNOB));
        addParam(createParamCentered<Trimpot>(themePos("GRAINS_ATTN_KNOB"), module, GrainEngineMK2::GRAINS_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("GRAINS_INPUT"), module, GrainEngineMK2::GRAINS_INPUT));

        // Pitch
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PITCH_KNOB"), module, GrainEngineMK2::PITCH_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("PITCH_INPUT"), module, GrainEngineMK2::PITCH_INPUT));

        // Sample
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("SAMPLE_KNOB"), module, GrainEngineMK2::SAMPLE_KNOB));
        addParam(createParamCentered<Trimpot>(themePos("SAMPLE_ATTN_KNOB"), module, GrainEngineMK2::SAMPLE_ATTN_KNOB));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("SAMPLE_INPUT"), module, GrainEngineMK2::SAMPLE_INPUT));

        // Spawn trigger input
        addInput(createInputCentered<VoxglitchInputPort>(themePos("SPAWN_TRIGGER_INPUT"), module, GrainEngineMK2::SPAWN_TRIGGER_INPUT));

        // Pan input
        addInput(createInputCentered<VoxglitchInputPort>(themePos("PAN_INPUT"), module, GrainEngineMK2::PAN_INPUT));

        // Trim knob
        addParam(createParamCentered<Trimpot>(themePos("TRIM_KNOB"), module, GrainEngineMK2::TRIM_KNOB));

        // Audio Output
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_LEFT"), module, GrainEngineMK2::AUDIO_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_RIGHT"), module, GrainEngineMK2::AUDIO_OUTPUT_RIGHT));

        GrainEngineMK2PosDisplay *pos_display = new GrainEngineMK2PosDisplay();
        pos_display->box.pos = themePos("DISPLAY");
        pos_display->module = module;
        addChild(pos_display);
    }

    /*
    struct BipolarPitchOption : MenuItem {
      GrainEngineMK2 *module;

      void onAction(const event::Action &e) override {
        module->bipolar_pitch_mode ^= true; // flip the value
      }
    };
    */

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
