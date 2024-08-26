struct SamplerX8Widget : VoxglitchSamplerModuleWidget
{
    SamplerX8Widget(SamplerX8 *module)
    {
        setModule(module);
        // setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/samplerx8_front_panel.svg")));

        // Load and apply theme
        theme.load("samplerx8");
        applyTheme();

        // setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/samplerx8/panel.svg")));

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/samplerx8/samplerx8_panel.svg"),
            asset::plugin(pluginInstance, "res/samplerx8/samplerx8_panel-dark.svg")
        ));

        if (theme.showScrews() == true)
        {
            addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        }

        // Add trigger inputs
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIGGER_INPUTS_0"), module, SamplerX8::TRIGGER_INPUTS + 0));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIGGER_INPUTS_1"), module, SamplerX8::TRIGGER_INPUTS + 1));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIGGER_INPUTS_2"), module, SamplerX8::TRIGGER_INPUTS + 2));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIGGER_INPUTS_3"), module, SamplerX8::TRIGGER_INPUTS + 3));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIGGER_INPUTS_4"), module, SamplerX8::TRIGGER_INPUTS + 4));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIGGER_INPUTS_5"), module, SamplerX8::TRIGGER_INPUTS + 5));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIGGER_INPUTS_6"), module, SamplerX8::TRIGGER_INPUTS + 6));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIGGER_INPUTS_7"), module, SamplerX8::TRIGGER_INPUTS + 7));

        // Add position inputs
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUTS_0"), module, SamplerX8::POSITION_INPUTS + 0));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUTS_1"), module, SamplerX8::POSITION_INPUTS + 1));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUTS_2"), module, SamplerX8::POSITION_INPUTS + 2));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUTS_3"), module, SamplerX8::POSITION_INPUTS + 3));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUTS_4"), module, SamplerX8::POSITION_INPUTS + 4));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUTS_5"), module, SamplerX8::POSITION_INPUTS + 5));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUTS_6"), module, SamplerX8::POSITION_INPUTS + 6));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUTS_7"), module, SamplerX8::POSITION_INPUTS + 7));

        // Add volume knobs
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("VOLUME_KNOBS_0"), module, SamplerX8::VOLUME_KNOBS + 0));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("VOLUME_KNOBS_1"), module, SamplerX8::VOLUME_KNOBS + 1));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("VOLUME_KNOBS_2"), module, SamplerX8::VOLUME_KNOBS + 2));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("VOLUME_KNOBS_3"), module, SamplerX8::VOLUME_KNOBS + 3));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("VOLUME_KNOBS_4"), module, SamplerX8::VOLUME_KNOBS + 4));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("VOLUME_KNOBS_5"), module, SamplerX8::VOLUME_KNOBS + 5));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("VOLUME_KNOBS_6"), module, SamplerX8::VOLUME_KNOBS + 6));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("VOLUME_KNOBS_7"), module, SamplerX8::VOLUME_KNOBS + 7));

        // Add pan knobs
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PAN_KNOBS_0"), module, SamplerX8::PAN_KNOBS + 0));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PAN_KNOBS_1"), module, SamplerX8::PAN_KNOBS + 1));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PAN_KNOBS_2"), module, SamplerX8::PAN_KNOBS + 2));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PAN_KNOBS_3"), module, SamplerX8::PAN_KNOBS + 3));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PAN_KNOBS_4"), module, SamplerX8::PAN_KNOBS + 4));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PAN_KNOBS_5"), module, SamplerX8::PAN_KNOBS + 5));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PAN_KNOBS_6"), module, SamplerX8::PAN_KNOBS + 6));
        addParam(createParamCentered<RoundLargeBlackKnob>(themePos("PAN_KNOBS_7"), module, SamplerX8::PAN_KNOBS + 7));

        addParam(createParamCentered<squareToggle>(themePos("MUTE_BUTTONS_0"), module, SamplerX8::MUTE_BUTTONS + 0));
        addParam(createParamCentered<squareToggle>(themePos("MUTE_BUTTONS_1"), module, SamplerX8::MUTE_BUTTONS + 1));
        addParam(createParamCentered<squareToggle>(themePos("MUTE_BUTTONS_2"), module, SamplerX8::MUTE_BUTTONS + 2));
        addParam(createParamCentered<squareToggle>(themePos("MUTE_BUTTONS_3"), module, SamplerX8::MUTE_BUTTONS + 3));
        addParam(createParamCentered<squareToggle>(themePos("MUTE_BUTTONS_4"), module, SamplerX8::MUTE_BUTTONS + 4));
        addParam(createParamCentered<squareToggle>(themePos("MUTE_BUTTONS_5"), module, SamplerX8::MUTE_BUTTONS + 5));
        addParam(createParamCentered<squareToggle>(themePos("MUTE_BUTTONS_6"), module, SamplerX8::MUTE_BUTTONS + 6));
        addParam(createParamCentered<squareToggle>(themePos("MUTE_BUTTONS_7"), module, SamplerX8::MUTE_BUTTONS + 7));

        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_LEFT_OUTPUTS_0"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 0));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_LEFT_OUTPUTS_1"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 1));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_LEFT_OUTPUTS_2"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 2));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_LEFT_OUTPUTS_3"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 3));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_LEFT_OUTPUTS_4"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 4));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_LEFT_OUTPUTS_5"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 5));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_LEFT_OUTPUTS_6"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 6));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_LEFT_OUTPUTS_7"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 7));

        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_RIGHT_OUTPUTS_0"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 0));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_RIGHT_OUTPUTS_1"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 1));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_RIGHT_OUTPUTS_2"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 2));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_RIGHT_OUTPUTS_3"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 3));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_RIGHT_OUTPUTS_4"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 4));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_RIGHT_OUTPUTS_5"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 5));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_RIGHT_OUTPUTS_6"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 6));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_RIGHT_OUTPUTS_7"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 7));

        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_MIX_OUTPUT_LEFT"), module, SamplerX8::AUDIO_MIX_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_MIX_OUTPUT_RIGHT"), module, SamplerX8::AUDIO_MIX_OUTPUT_RIGHT));
    }

    void appendContextMenu(Menu *menu) override
    {
        SamplerX8 *module = dynamic_cast<SamplerX8 *>(this->module);
        assert(module);

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Load individual samples"));

        for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
        {
            SamplerX8LoadSample *menu_item_load_sample = new SamplerX8LoadSample();
            menu_item_load_sample->sample_number = i;
            menu_item_load_sample->text = std::to_string(i + 1) + ": " + module->loaded_filenames[i];
            menu_item_load_sample->module = module;
            menu->addChild(menu_item_load_sample);
        }

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Or.."));

        SamplerX8LoadFolder *menu_item_load_folder = new SamplerX8LoadFolder();
        menu_item_load_folder->text = "Load first 8 WAV files from a folder";
        menu_item_load_folder->module = module;
        menu->addChild(menu_item_load_folder);

        // Interpolation menu
        menu->addChild(new MenuEntry); // For spacing only
        SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
        sample_interpolation_menu_item->module = module;
        menu->addChild(sample_interpolation_menu_item);
    }
};
