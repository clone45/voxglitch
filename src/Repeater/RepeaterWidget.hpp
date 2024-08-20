struct RepeaterWidget : VoxglitchSamplerModuleWidget
{
    RepeaterWidget(Repeater *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("repeater");
        applyTheme();

        // setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/repeater/panel.svg")));

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/repeater/repeater_panel.svg"),
            asset::plugin(pluginInstance, "res/repeater/repeater_panel-dark.svg")
        ));

        // Add rack screws
        if (theme.showScrews())
        {
            addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewHexBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewHexBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        }

        // Medium Knobs
        addParam(createParamCentered<VoxglitchMediumKnob>(themePos("CLOCK_DIVISION_KNOB"), module, Repeater::CLOCK_DIVISION_KNOB));
        addParam(createParamCentered<VoxglitchMediumKnob>(themePos("POSITION_KNOB"), module, Repeater::POSITION_KNOB));
        addParam(createParamCentered<VoxglitchMediumKnob>(themePos("SAMPLE_SELECT_KNOB"), module, Repeater::SAMPLE_SELECT_KNOB));
        addParam(createParamCentered<VoxglitchMediumKnob>(themePos("PITCH_KNOB"), module, Repeater::PITCH_KNOB));

        // CV Inputs
        addInput(createInputCentered<VoxglitchInputPort>(themePos("CLOCK_DIVISION_INPUT"), module, Repeater::CLOCK_DIVISION_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_INPUT"), module, Repeater::POSITION_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("SAMPLE_SELECT_INPUT"), module, Repeater::SAMPLE_SELECT_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(themePos("PITCH_INPUT"), module, Repeater::PITCH_INPUT));

        // Attenuators
        addParam(createParamCentered<VoxglitchAttenuator>(themePos("CLOCK_DIVISION_ATTN_KNOB"), module, Repeater::CLOCK_DIVISION_ATTN_KNOB));
        addParam(createParamCentered<VoxglitchAttenuator>(themePos("POSITION_ATTN_KNOB"), module, Repeater::POSITION_ATTN_KNOB));
        addParam(createParamCentered<VoxglitchAttenuator>(themePos("SAMPLE_SELECT_ATTN_KNOB"), module, Repeater::SAMPLE_SELECT_ATTN_KNOB));
        // addParam(createParamCentered<VoxglitchAttenuator>(Vec(83.563423,331.700439), module, Repeater::PITCH_ATTN_KNOB));

        // Clock input
        addInput(createInputCentered<VoxglitchInputPort>(themePos("TRIG_INPUT"), module, Repeater::TRIG_INPUT));

        // Smooth switch
        addParam(createParamCentered<squareToggle>(themePos("SMOOTH_SWITCH"), module, Repeater::SMOOTH_SWITCH));

        // Outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("WAV_OUTPUT"), module, Repeater::WAV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("TRG_OUTPUT"), module, Repeater::TRG_OUTPUT));

        Readout *readout = new Readout();
        readout->box.pos = themePos("READOUT"); // 22,22
        readout->box.size = Vec(110, 30);       // bounding box of the widget
        readout->module = module;
        addChild(readout);
    }

    void appendContextMenu(Menu *menu) override
    {
        Repeater *module = dynamic_cast<Repeater *>(this->module);
        assert(module);

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Samples"));

        //
        // Add the five "Load Sample.." menu options to the right-click context menu
        //

        for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
        {
            MenuItemLoadSample *menu_item_load_sample = new MenuItemLoadSample;
            menu_item_load_sample->sample_number = i;
            menu_item_load_sample->text = std::to_string(i + 1) + ": " + module->loaded_filenames[i];
            menu_item_load_sample->module = module;
            menu->addChild(menu_item_load_sample);
        }

        //
        // Options
        // =====================================================================

        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("Options"));

        // Retrigger option

        struct RetriggerMenuItem : MenuItem
        {
            Repeater *module;
            void onAction(const event::Action &e) override
            {
                module->retrigger = !(module->retrigger);
            }
        };

        RetriggerMenuItem *retrigger_menu_item = createMenuItem<RetriggerMenuItem>("Retrigger");
        retrigger_menu_item->rightText = CHECKMARK(module->retrigger == 1);
        retrigger_menu_item->module = module;
        menu->addChild(retrigger_menu_item);

        // Sample interpolation settings
        SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
        sample_interpolation_menu_item->module = module;
        menu->addChild(sample_interpolation_menu_item);
    }
};
