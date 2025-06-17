struct RepeaterWidget : VoxglitchSamplerModuleWidget
{
    RepeaterWidget(Repeater *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/repeater/repeater_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/repeater/repeater_panel-dark.svg")
        );

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


        // Medium Knobs
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("div_knob"), module, Repeater::CLOCK_DIVISION_KNOB));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pos_knob"), module, Repeater::POSITION_KNOB));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("wav_knob"), module, Repeater::SAMPLE_SELECT_KNOB));
        addParam(createParamCentered<RoundLargeBlackKnob>(panelHelper.findNamed("pitch_knob"), module, Repeater::PITCH_KNOB));

        // CV Inputs
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("div_input"), module, Repeater::CLOCK_DIVISION_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pos_input"), module, Repeater::POSITION_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("wav_input"), module, Repeater::SAMPLE_SELECT_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("pitch_input"), module, Repeater::PITCH_INPUT));

        // Attenuators
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("div_attn_knob"), module, Repeater::CLOCK_DIVISION_ATTN_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("pos_attn_knob"), module, Repeater::POSITION_ATTN_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("wav_attn_knob"), module, Repeater::SAMPLE_SELECT_ATTN_KNOB));
        // addParam(createParamCentered<Trimpot>(Vec(83.563423,331.700439), module, Repeater::PITCH_ATTN_KNOB));

        // Clock input
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("clock_input"), module, Repeater::TRIG_INPUT));

        // Smooth switch
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("smooth_button"), module, Repeater::SMOOTH_SWITCH));

        // Outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("wav_output"), module, Repeater::WAV_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("trg_output"), module, Repeater::TRG_OUTPUT));

        Readout *readout = new Readout();
        readout->box.pos = Vec(29.5276, 57.5787);
        readout->box.size = Vec(110, 30);
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
