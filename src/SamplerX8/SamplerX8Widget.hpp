struct SamplerX8Widget : VoxglitchSamplerModuleWidget
{
    SamplerX8Widget(SamplerX8 *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("samplerx8");
        applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/samplerx8/samplerx8_panel.svg"),
            asset::plugin(pluginInstance, "res/samplerx8/samplerx8_panel-dark.svg")
        ));

        // Add volume knobs
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(73.6109, 69.3359), module, SamplerX8::VOLUME_KNOBS + 0));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(131.4259, 69.3359), module, SamplerX8::VOLUME_KNOBS + 1));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(189.2408, 69.3359), module, SamplerX8::VOLUME_KNOBS + 2));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(247.0385, 69.3359), module, SamplerX8::VOLUME_KNOBS + 3));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(304.9398, 69.3359), module, SamplerX8::VOLUME_KNOBS + 4));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(362.6684, 69.3359), module, SamplerX8::VOLUME_KNOBS + 5));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(420.3971, 69.3359), module, SamplerX8::VOLUME_KNOBS + 6));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(478.402, 69.3359), module, SamplerX8::VOLUME_KNOBS + 7));

        // Add pan knobs
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(73.6109, 119.5313), module, SamplerX8::PAN_KNOBS + 0));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(131.4259, 119.5313), module, SamplerX8::PAN_KNOBS + 1));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(189.2408, 119.5313), module, SamplerX8::PAN_KNOBS + 2));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(247.0385, 119.5313), module, SamplerX8::PAN_KNOBS + 3));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(304.9398, 119.5313), module, SamplerX8::PAN_KNOBS + 4));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(362.6684, 119.5313), module, SamplerX8::PAN_KNOBS + 5));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(420.3971, 119.5313), module, SamplerX8::PAN_KNOBS + 6));
        addParam(createParamCentered<RoundLargeBlackKnob>(Vec(478.402, 119.5313), module, SamplerX8::PAN_KNOBS + 7));

        // Add position inputs
        addInput(createInputCentered<VoxglitchInputPort>(Vec(73.6109, 170.8984), module, SamplerX8::POSITION_INPUTS + 0));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(131.4259, 170.8984), module, SamplerX8::POSITION_INPUTS + 1));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(189.2408, 170.8984), module, SamplerX8::POSITION_INPUTS + 2));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(247.0385, 170.8984), module, SamplerX8::POSITION_INPUTS + 3));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(304.9398, 170.8984), module, SamplerX8::POSITION_INPUTS + 4));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(362.6684, 170.8984), module, SamplerX8::POSITION_INPUTS + 5));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(420.3971, 170.8984), module, SamplerX8::POSITION_INPUTS + 6));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(478.402, 170.8984), module, SamplerX8::POSITION_INPUTS + 7));

        // Add trigger inputs
        addInput(createInputCentered<VoxglitchInputPort>(Vec(73.6109, 214.4531), module, SamplerX8::TRIGGER_INPUTS + 0));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(131.4259, 214.4531), module, SamplerX8::TRIGGER_INPUTS + 1));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(189.2408, 214.4531), module, SamplerX8::TRIGGER_INPUTS + 2));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(247.0385, 214.4531), module, SamplerX8::TRIGGER_INPUTS + 3));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(304.9398, 214.4531), module, SamplerX8::TRIGGER_INPUTS + 4));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(362.6684, 214.4531), module, SamplerX8::TRIGGER_INPUTS + 5));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(420.3971, 214.4531), module, SamplerX8::TRIGGER_INPUTS + 6));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(478.402, 214.4531), module, SamplerX8::TRIGGER_INPUTS + 7));

        // Add mute buttons
        addParam(createParamCentered<squareToggle>(Vec(73.6109, 257.6172), module, SamplerX8::MUTE_BUTTONS + 0));
        addParam(createParamCentered<squareToggle>(Vec(131.4259, 257.6172), module, SamplerX8::MUTE_BUTTONS + 1));
        addParam(createParamCentered<squareToggle>(Vec(189.2408, 257.6172), module, SamplerX8::MUTE_BUTTONS + 2));
        addParam(createParamCentered<squareToggle>(Vec(247.0385, 257.6172), module, SamplerX8::MUTE_BUTTONS + 3));
        addParam(createParamCentered<squareToggle>(Vec(304.9398, 257.6172), module, SamplerX8::MUTE_BUTTONS + 4));
        addParam(createParamCentered<squareToggle>(Vec(362.6684, 257.6172), module, SamplerX8::MUTE_BUTTONS + 5));
        addParam(createParamCentered<squareToggle>(Vec(420.3971, 257.6172), module, SamplerX8::MUTE_BUTTONS + 6));
        addParam(createParamCentered<squareToggle>(Vec(478.402, 257.6172), module, SamplerX8::MUTE_BUTTONS + 7));

        // Add left outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(73.6109, 318.1981), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 0));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(131.4259, 318.1981), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 1));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(189.2408, 318.1981), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 2));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(247.0385, 318.1981), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 3));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(304.9398, 318.1981), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 4));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(362.6684, 318.1981), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 5));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(420.3971, 318.1981), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 6));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(478.402, 318.1981), module, SamplerX8::AUDIO_LEFT_OUTPUTS + 7));

        // Add right outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(73.6109, 352.4485), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 0));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(131.4259, 352.4485), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 1));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(189.2408, 352.4485), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 2));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(247.0385, 352.4485), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 3));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(304.9398, 352.4485), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 4));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(362.6684, 352.4485), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 5));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(420.3971, 352.4485), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 6));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(478.402, 352.4485), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + 7));

        // Add mix outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(536.2687, 318.1981), module, SamplerX8::AUDIO_MIX_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(536.2687, 352.4485), module, SamplerX8::AUDIO_MIX_OUTPUT_RIGHT));
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
