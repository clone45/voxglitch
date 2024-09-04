struct SamplerX8Widget : VoxglitchSamplerModuleWidget
{
    SamplerX8Widget(SamplerX8 *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/samplerx8/samplerx8_panel.svg"),
            asset::plugin(pluginInstance, "res/samplerx8/samplerx8_panel-dark.svg")
        );
        // Add volume knobs
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("volume_knob_1"), module, SamplerX8::VOLUME_KNOBS + 0));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("volume_knob_2"), module, SamplerX8::VOLUME_KNOBS + 1));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("volume_knob_3"), module, SamplerX8::VOLUME_KNOBS + 2));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("volume_knob_4"), module, SamplerX8::VOLUME_KNOBS + 3));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("volume_knob_5"), module, SamplerX8::VOLUME_KNOBS + 4));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("volume_knob_6"), module, SamplerX8::VOLUME_KNOBS + 5));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("volume_knob_7"), module, SamplerX8::VOLUME_KNOBS + 6));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("volume_knob_8"), module, SamplerX8::VOLUME_KNOBS + 7));

        // Add pan knobs
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pan_knob_1"), module, SamplerX8::PAN_KNOBS + 0));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pan_knob_2"), module, SamplerX8::PAN_KNOBS + 1));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pan_knob_3"), module, SamplerX8::PAN_KNOBS + 2));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pan_knob_4"), module, SamplerX8::PAN_KNOBS + 3));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pan_knob_5"), module, SamplerX8::PAN_KNOBS + 4));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pan_knob_6"), module, SamplerX8::PAN_KNOBS + 5));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pan_knob_7"), module, SamplerX8::PAN_KNOBS + 6));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pan_knob_8"), module, SamplerX8::PAN_KNOBS + 7));

        // Add position inputs
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pos_input_1"), module, SamplerX8::POSITION_INPUTS + 0));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pos_input_2"), module, SamplerX8::POSITION_INPUTS + 1));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pos_input_3"), module, SamplerX8::POSITION_INPUTS + 2));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pos_input_4"), module, SamplerX8::POSITION_INPUTS + 3));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pos_input_5"), module, SamplerX8::POSITION_INPUTS + 4));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pos_input_6"), module, SamplerX8::POSITION_INPUTS + 5));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pos_input_7"), module, SamplerX8::POSITION_INPUTS + 6));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pos_input_8"), module, SamplerX8::POSITION_INPUTS + 7));

        // Add trigger inputs
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input_1"), module, SamplerX8::TRIGGER_INPUTS + 0));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input_2"), module, SamplerX8::TRIGGER_INPUTS + 1));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input_3"), module, SamplerX8::TRIGGER_INPUTS + 2));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input_4"), module, SamplerX8::TRIGGER_INPUTS + 3));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input_5"), module, SamplerX8::TRIGGER_INPUTS + 4));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input_6"), module, SamplerX8::TRIGGER_INPUTS + 5));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input_7"), module, SamplerX8::TRIGGER_INPUTS + 6));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("trig_input_8"), module, SamplerX8::TRIGGER_INPUTS + 7));

        // Add mute buttons
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("mute_button_1"), module, SamplerX8::MUTE_BUTTONS + 0));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("mute_button_2"), module, SamplerX8::MUTE_BUTTONS + 1));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("mute_button_3"), module, SamplerX8::MUTE_BUTTONS + 2));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("mute_button_4"), module, SamplerX8::MUTE_BUTTONS + 3));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("mute_button_5"), module, SamplerX8::MUTE_BUTTONS + 4));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("mute_button_6"), module, SamplerX8::MUTE_BUTTONS + 5));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("mute_button_7"), module, SamplerX8::MUTE_BUTTONS + 6));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("mute_button_8"), module, SamplerX8::MUTE_BUTTONS + 7));

        // Add left outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output_1"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 0));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output_2"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 1));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output_3"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 2));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output_4"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 3));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output_5"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 4));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output_6"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 5));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output_7"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 6));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output_8"), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 7));

        // Add right outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output_1"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 0));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output_2"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 1));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output_3"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 2));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output_4"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 3));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output_5"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 4));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output_6"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 5));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output_7"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 6));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output_8"), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 7));

        // Add mix outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("mix_output_left"), module, SamplerX8::AUDIO_MIX_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("mix_output_right"), module, SamplerX8::AUDIO_MIX_OUTPUT_RIGHT));
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
